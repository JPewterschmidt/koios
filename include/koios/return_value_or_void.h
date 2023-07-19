#ifndef KOIOS_RETURN_VALUE_OR_VOID_H
#define KOIOS_RETURN_VALUE_OR_VOID_H

#include <future>
#include <coroutine>
#include <memory>
#include <utility>

#include "koios/macros.h"

KOIOS_NAMESPACE_BEG

/*! \brief store the return value. Wake this caller coroutine.
 *
 *  \tparam T the return value type just like the one of a regular function.
 *  \tparam Promise the promise type of inherited class.
 *  \tparam The DriverPolicy of the promise type of inherited class.
 */
template<typename T, typename Promise, typename DriverPolicy>
class return_value_or_void_base 
{
public:
    void set_caller(::std::coroutine_handle<> h) noexcept { m_caller = h; }
    auto get_future() { return m_promise_p->get_future(); }
    auto get_std_promise_pointer() { return m_promise_p; }

protected:
    ::std::shared_ptr<::std::promise<T>> m_promise_p{ new ::std::promise<T>{} };
    ::std::coroutine_handle<> m_caller{};

    void wake_caller()
    {
        if (!m_caller) return;
        DriverPolicy{}.scheduler().enqueue(m_caller);
    }
};

template<typename T, typename Promise, typename DriverPolicy>
class return_value_or_void 
    : public return_value_or_void_base<T, Promise, DriverPolicy>
{
public: 
    template<typename TT>
    requires (::std::constructible_from<T, TT>)
    void return_value(TT&& val)
    {
        return_value_or_void_base<T, Promise, DriverPolicy>::
            m_promise_p->set_value(::std::forward<TT>(val));
        return_value_or_void_base<T, Promise, DriverPolicy>::wake_caller();
    }
};

template<typename Promise, typename DriverPolicy>
class return_value_or_void<void, Promise, DriverPolicy> 
    : public return_value_or_void_base<void, Promise, DriverPolicy>
{
public:
    void return_void() 
    { 
        return_value_or_void_base<void, Promise, DriverPolicy>::m_promise_p->set_value(); 
        return_value_or_void_base<void, Promise, DriverPolicy>::wake_caller(); 
    }
};

KOIOS_NAMESPACE_END

#endif
