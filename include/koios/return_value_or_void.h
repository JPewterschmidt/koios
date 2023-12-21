#ifndef KOIOS_RETURN_VALUE_OR_VOID_H
#define KOIOS_RETURN_VALUE_OR_VOID_H

#include <future>
#include <coroutine>
#include <memory>
#include <utility>

#include "koios/macros.h"
#include "koios/task_on_the_fly.h"
#include "koios/future.h"

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
    /*! Set the caller coroutine handle which coroutine represented will be wake up after this task done.  */
    void set_caller(task_on_the_fly h) noexcept { m_caller = ::std::move(h); } 

    /*! Take the ownership of the future object */
    auto get_future() { return m_promise_p->get_future(); }
    auto get_std_promise_pointer() { return m_promise_p; }

protected:
    ::std::shared_ptr<koios::promise<T>> m_promise_p{ ::std::make_shared<koios::promise<T>>() };
    task_on_the_fly m_caller{};

    /*! \brief Wake the caller coroutine, if this task has been called with `co_await`.
     *  If this task was scheduled by `task_scheduler` directly, this function won't do anything.
     */
    void wake_caller()
    {
        if (!m_caller) return;
        DriverPolicy{}.scheduler().enqueue(::std::move(m_caller));
    }

    void deal_exception(::std::exception_ptr ep)
    {
        m_promise_p->set_exception(::std::move(ep));
        wake_caller();
    }
};

/*! \brief The major `return_value_or_void` class template.
 *  \tparam T the return type of the task.
 *  \tparam Promise the promise type of the `task`.
 *  \tparam DriverPolicy the driver policy type used when waking up the caller coroutine.
 */
template<typename T, typename Promise, typename DriverPolicy>
class return_value_or_void 
    : public return_value_or_void_base<T, Promise, DriverPolicy>
{
public: 
    /*! \brief set the return value.
     *  The compiler generates code which call this function when user `co_return` something.
     *  After store the return value, this function will wake up the caller, 
     *  if there's a caller task call this task by `co_await`.
     */
    template<typename TT>
    requires (::std::constructible_from<T, TT>)
    void return_value(TT&& val)
    {
        this->m_promise_p->set_value(::std::forward<TT>(val));
        this->wake_caller();
    }
};

/*! \brief the specializtion of return_value_or_void, which deal with void return type. */
template<typename Promise, typename DriverPolicy>
class return_value_or_void<void, Promise, DriverPolicy> 
    : public return_value_or_void_base<void, Promise, DriverPolicy>
{
public:
    /*! \brief Just wake the caller. */
    void return_void() 
    { 
        this->m_promise_p->set_value(); 
        this->wake_caller(); 
    }
};

KOIOS_NAMESPACE_END

#endif
