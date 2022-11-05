#pragma once

#include <utility>

#include <boost/log/attributes/mutable_constant.hpp>
#include <boost/log/core/record.hpp>
#include <boost/log/utility/strictest_lock.hpp>
#include <boost/parameter/aux_/void.hpp>
#include <boost/parameter/keyword.hpp>

#include "logger/detail/attributes.h"

namespace logger::detail
{
namespace keywords
{
    BOOST_PARAMETER_KEYWORD(tags, NumId)
}    // namespace keywords

template <typename BaseT>
class NumIdFeature : public BaseT
{
public:
    using base_type = BaseT;
    BOOST_COPYABLE_AND_MOVABLE_ALT(NumIdFeature)
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
    NumIdFeature() : base_type{}, m_numIdAttr{ attributes::num_id_t{} }
    {
        base_type::add_attribute_unlocked(attributes::kNumId, m_numIdAttr);
    }

    NumIdFeature(const NumIdFeature &rhs)
        : base_type(rhs), m_numIdAttr(rhs.m_numIdAttr.get())
    {
        base_type::attributes()[attributes::kNumId] = m_numIdAttr;
    }

    NumIdFeature(NumIdFeature &&rhs)
        : base_type(std::move(rhs)), m_numIdAttr(std::move(rhs.m_numIdAttr))
    {
    }

    template <typename ArgsT>
    explicit NumIdFeature(const ArgsT &args)
        : base_type(args), m_numIdAttr(args[keywords::NumId || make_default_num_id{}])
    {
        base_type::add_attribute_unlocked(attributes::kNumId, m_numIdAttr);
    }

protected:
    template <typename ArgsT>
    boost::log::record open_record_unlocked(const ArgsT &args)
    {
        return open_record_with_id_unlocked(
            args, args[keywords::NumId | boost::parameter::void_()]);
    }

    /*!
     * Unlocked swap
     */
    void swap_unlocked(NumIdFeature &rhs)
    {
        base_type::swap_unlocked(static_cast<base_type &>(rhs));
        m_numIdAttr.swap(rhs.m_numIdAttr);
    }

private:
    struct make_default_num_id
    {
        using result_type = attributes::num_id_t;
        result_type operator()() const { return result_type{}; }
    };

    //! The implementation for the case when the num id is specified in log statement
    template <typename ArgsT, typename T>
    boost::log::record open_record_with_id_unlocked(const ArgsT &args, const T &numId)
    {
        m_numIdAttr.set(numId);
        return base_type::open_record_unlocked(args);
    }
    //! The implementation for the case when the num id is not specified in log statement
    template <typename ArgsT>
    boost::log::record open_record_with_id_unlocked(const ArgsT &args,
                                                    boost::parameter::void_)
    {
        return base_type::open_record_unlocked(args);
    }

    boost::log::attributes::mutable_constant<attributes::num_id_t> m_numIdAttr;
};

struct NumId
{
    template <typename BaseT>
    struct apply
    {
        using type = NumIdFeature<BaseT>;
    };
};

}    // namespace logger::detail
