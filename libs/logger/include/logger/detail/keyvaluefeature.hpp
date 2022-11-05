#pragma once

#include <boost/log/attributes/attribute_name.hpp>
#include <boost/log/attributes/mutable_constant.hpp>
#include <boost/log/core/record.hpp>
#include <boost/log/utility/strictest_lock.hpp>
#include <boost/parameter/aux_/void.hpp>
#include <boost/type_traits/is_nothrow_move_constructible.hpp>

namespace logger::detail
{
/**
 * @brief map the keyword into an attribute with a given value
 */
template <typename BaseT, typename KeywordGetter, typename AttrNameGetter, typename Value>
class KeyValueFeature : public BaseT
{
    using base_type = BaseT;
    using this_type = KeyValueFeature;
    BOOST_COPYABLE_AND_MOVABLE_ALT(this_type)

public:
    using char_type       = typename base_type::char_type;
    using final_type      = typename base_type::final_type;
    using threading_model = typename base_type::threading_model;

    using value_type      = Value;
    using value_attribute = boost::log::attributes::mutable_constant<value_type>;

    using open_record_lock = typename boost::log::strictest_lock<
        typename base_type::open_record_lock,
        boost::log::aux::exclusive_lock_guard<threading_model>>::type;

    using swap_lock = typename boost::log::strictest_lock<
        typename base_type::swap_lock,
        boost::log::aux::multiple_unique_lock2<threading_model, threading_model>>::type;

private:
    struct make_default_value
    {
        using result_type = value_type;
        result_type operator()() const { return result_type{}; }
    };

private:
    value_attribute m_valueAttr;

public:
    KeyValueFeature() : base_type(), m_valueAttr(value_type{})
    {
        base_type::add_attribute_unlocked(
            boost::log::attribute_name{ AttrNameGetter{}() }, m_valueAttr);
    }

    KeyValueFeature(const KeyValueFeature &that)
        : base_type(static_cast<const base_type &>(that)),
          m_valueAttr(that.m_valueAttr.get())
    {
        // Our attributes must refer to our value attribute
        base_type::attributes()[boost::log::attribute_name{ AttrNameGetter{}() }] =
            m_valueAttr;
    }

    KeyValueFeature(BOOST_RV_REF(KeyValueFeature) that) BOOST_NOEXCEPT_IF(
        boost::is_nothrow_move_constructible<base_type>::value
            &&boost::is_nothrow_move_constructible<value_attribute>::value)
        : base_type(boost::move(static_cast<base_type &>(that))),
          m_valueAttr(boost::move(that.m_valueAttr))
    {
    }

    template <typename ArgsT>
    explicit KeyValueFeature(ArgsT const &args)
        : base_type(args), m_valueAttr(args[KeywordGetter{}() || make_default_value()])
    {
        base_type::add_attribute_unlocked(
            boost::log::attribute_name{ AttrNameGetter{}() }, m_valueAttr);
    }

protected:
    template <typename ArgsT>
    boost::log::record open_record_unlocked(ArgsT const &args)
    {
        return open_record_with_key_unlocked(
            args, args[KeywordGetter{}() | boost::parameter::void_()]);
    }

    void swap_unlocked(KeyValueFeature &that)
    {
        base_type::swap_unlocked(static_cast<base_type &>(that));
        m_valueAttr.swap(that.m_valueAttr);
    }

private:
    /**
     * @brief implementation for the case when the keyword is specified in log statement
     */
    template <typename ArgsT, typename T>
    boost::log::record open_record_with_key_unlocked(ArgsT const &args, T const &ch)
    {
        m_valueAttr.set(ch);
        return base_type::open_record_unlocked(args);
    }

    /**
     * @brief implementation for the case when the keyword is not specified in log
     * statement
     */
    template <typename ArgsT>
    boost::log::record open_record_with_key_unlocked(ArgsT const &args,
                                                     boost::parameter::void_)
    {
        return base_type::open_record_unlocked(args);
    }
};

template <typename KeywordGetter, typename AttrNameGetter, typename Value>
struct KeyValue
{
    template <typename BaseT>
    struct apply
    {
        using type = KeyValueFeature<BaseT, KeywordGetter, AttrNameGetter, Value>;
    };
};

}    // namespace logger::detail
