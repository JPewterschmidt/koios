/* Koios, A c++ async runtime library.
 * Copyright (C) 2024  Jeremy Pewterschmidt
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef KOIOS_RETURN_VALUE_OR_VOID_H
#define KOIOS_RETURN_VALUE_OR_VOID_H

#include <future>
#include <coroutine>
#include <memory>
#include <utility>
#include <exception>

#include "koios/macros.h"
#include "koios/task_on_the_fly.h"
#include "koios/future.h"

#include "toolpex/move_only.h"

KOIOS_NAMESPACE_BEG

/*! \brief store the return value. Wake this caller coroutine.
 *
 *  \tparam T the return value type just like the one of a regular function.
 *  \tparam Promise the promise type of inherited class.
 *  \tparam The DriverPolicy of the promise type of inherited class.
 */
template<typename T, typename Promise, typename DriverPolicy>
class return_value_or_void_base : public toolpex::move_only // See also https://devblogs.microsoft.com/oldnewthing/20210504-00/?p=105176
{
public:
    /*! Set the caller coroutine handle which coroutine represented will be wake up after this task done.  
     *
     *  The counterpart future (in practice `get_result_aw::await_suspend`)
     *  would call this function, Set it's coroutine_handler as the caller handler.
     *  Thus, this function should NOT acquire the lock of future/promise shared state lock.
     */
    void set_caller(task_on_the_fly h) noexcept { m_caller = ::std::move(h); } 

    /*! Take the ownership of the future object */
    auto get_future() { return m_promise_p->get_future(); }
    auto get_std_promise_pointer() { return m_promise_p; }

#ifdef KOIOS_DEBUG
#include "koios/runtime.h"
    ~return_value_or_void_base() noexcept
    {
        if (!caller_woke_or_exception_caught() && !get_task_scheduler().is_cleaning() && !::std::current_exception()) [[unlikely]]
        {
            ::std::cerr << "You have to call `co_return`!!!!" << ::std::endl;
            ::std::terminate();
        }
    }
#endif

protected:
    ::std::shared_ptr<koios::promise<T>> m_promise_p{ ::std::make_shared<koios::promise<T>>() };
    task_on_the_fly m_caller{};

    /*! \brief Wake the caller coroutine, if this task has been called with `co_await`.
     *  If this task was scheduled by `task_scheduler` directly, this function won't do anything.
     *
     *  \retval true There is a handler point to the caller, and successfully wake it up.
     *  \retval false There is not a handler point to the caller, do nothing.
     */
    void wake_caller() noexcept
    {
#ifdef KOIOS_DEBUG
        m_caller_woke_or_exception_caught = true;
#endif
        if (!has_caller()) return;
        DriverPolicy{}.scheduler().enqueue(::std::move(m_caller));
    }

    bool has_caller() const noexcept { return !!m_caller; }

    // Called by `unhandled_exception`
    void deal_exception(::std::exception_ptr ep)
    {
#ifdef KOIOS_DEBUG
        m_caller_woke_or_exception_caught = true;
#endif
/**************************************************************************************/
/***                          Critical Section Begin                                ***/
/**************************************************************************************/
/**/    auto lk = m_promise_p->get_shared_state_lock();                             /**/
/**/    m_promise_p->set_exception(lk, ep);                                         /**/
/**/    if (has_caller())                                                           /**/
/**/    {                                                                           /**/
/**/        wake_caller();                                                          /**/
/**/    }                                                                           /**/
/**/    else                                                                        /**/
/**/    {                                                                           /**/
/**/        // For those tasks were activated by `t.run()`                          /**/
/**/        // or something make the task object never record the caller hander.    /**/
/**/        // We should let the exception handler of some layer                    /**/
/**/        // (such as `thread_pool`) know and do something,                       /**/
/**/        // or the programmer won't receive any information !                    /**/
/**/        ::std::rethrow_exception(ep);                                           /**/
/**/    }                                                                           /**/
/**************************************************************************************/
/***                           Critical Section End                                 ***/
/**************************************************************************************/
    }
#ifdef KOIOS_DEBUG
private:
    bool caller_woke_or_exception_caught() const noexcept { return m_caller_woke_or_exception_caught; }
    bool m_caller_woke_or_exception_caught{ false };
#endif
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
    template<::std::convertible_to<T> TT = T>
    void return_value(TT&& val)
    {
/**************************************************************************************/
/***                          Critical Section Begin                                ***/
/**************************************************************************************/
/**/    auto lk = this->m_promise_p->get_shared_state_lock();                       /**/
/**/    this->m_promise_p->set_value(lk, ::std::forward<TT>(val));                  /**/
/**/    this->wake_caller();                                                        /**/
/**************************************************************************************/
/***                           Critical Section End                                 ***/
/**************************************************************************************/
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
/**************************************************************************************/
/***                          Critical Section Begin                                ***/
/**************************************************************************************/
/**/    auto lk = this->m_promise_p->get_shared_state_lock();                       /**/
/**/    this->m_promise_p->set_value(lk);                                           /**/
/**/    this->wake_caller();                                                        /**/
/**************************************************************************************/
/***                           Critical Section End                                 ***/
/**************************************************************************************/
    }
};

KOIOS_NAMESPACE_END

#endif
