#pragma once

#include <utility>

#include <boost/log/attributes/mutable_constant.hpp>
#include <boost/log/core/record.hpp>
#include <boost/log/utility/strictest_lock.hpp>
#include <boost/parameter/aux_/void.hpp>

namespace logger::detail
{
template <typename BaseT, typename KeywordGetter, typename AttrNameGetter, typename Value>
class KeyValueFeature : public BaseT
{
public:
    using base_type = BaseT;
    using self_type = KeyValueFeature<BaseT, KeywordGetter, AttrNameGetter, Value>;
    BOOST_COPYABLE_AND_MOVABLE_ALT(KeyValueFeature)
    using char_type       = typename base_type::char_type;
    using final_type      = typename base_type::final_type;
    using threading_model = typename base_type::threading_model;

    using open_record_lock = typename boost::log::strictest_lock<
        typename base_type::open_record_lock,
        boost::log::aux::exclusive_lock_guard<threading_model>>::type;

    using swap_lock = typename boost::log::strictest_lock<
        typename base_type::swap_lock,
        boost::log::aux::multiple_unique_lock2<threading_model, threading_model>>::type;

public:
    KeyValueFeature() : base_type{}, m_attr{ Value{} }
    {
        base_type::add_attribute_unlocked(KeywordGetter{}(), m_attr);
    }

    KeyValueFeature(const self_type &rhs) : base_type{ rhs }, m_attr{ rhs.m_attr.get() }
    {
        base_type::attributes()[AttrNameGetter{}()] = m_attr;
    }

    KeyValueFeature(self_type &&rhs)
        : base_type{ std::move(rhs) }, m_attr{ std::move(rhs.m_attr) }
    {
    }

    template <typename ArgsT>
    explicit KeyValueFeature(const ArgsT &args)
        : base_type{ args },
          m_attr{ args[KeywordGetter{}() || make_default_attr_value{}] }
    {
        base_type::add_attribute_unlocked(AttrNameGetter{}(), m_attr);
    }

protected:
    template <typename ArgsT>
    boost::log::record open_record_unlocked(const ArgsT &args)
    {
        return open_record_with_attr_unlocked(
            args, args[KeywordGetter{}() | boost::parameter::void_()]);
    }

    /*!
     * Unlocked swap
     */
    void swap_unlocked(KeyValueFeature &rhs)
    {
        base_type::swap_unlocked(static_cast<base_type &>(rhs));
        m_attr.swap(rhs.m_attr);
    }

private:
    struct make_default_attr_value
    {
        using result_type = Value;
        result_type operator()() const { return result_type{}; }
    };

    //! The implementation for the case when keyword specified in log statement
    template <typename ArgsT, typename T>
    boost::log::record open_record_with_attr_unlocked(const ArgsT &args, const T &value)
    {
        m_attr.set(value);
        return base_type::open_record_unlocked(args);
    }
    //! The implementation for the case when keyword is not specified in log statement
    template <typename ArgsT>
    boost::log::record open_record_with_attr_unlocked(const ArgsT &args,
                                                      boost::parameter::void_)
    {
        return base_type::open_record_unlocked(args);
    }

    boost::log::attributes::mutable_constant<Value> m_attr;
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
