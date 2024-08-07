// This file is part of Koios
// https://github.com/JPewterschmidt/koios
//
// Copyleft 2023 - 2024, ShiXin Wang. All wrongs reserved.

#ifndef KOIOS_GET_RESULT_AW_H
#define KOIOS_GET_RESULT_AW_H

#include <future>
#include <memory>

#include "toolpex/assert.h"

#include "koios/macros.h"
#include "koios/local_thread_scheduler.h"
#include "koios/task_scheduler_wrapper.h"
#include "koios/task_on_the_fly.h"
#include "koios/future.h"

KOIOS_NAMESPACE_BEG

/*! \brief Awaitable base class for coroutine call in another coroutine task.
 *  
 *  Contains the requried member function which needed by the compiler,
 *  also holds the ownership of future.
 *
 *  \tparam T result type of the current task.
 *  \tparam Task `task` type of the current task.
 */
template<typename T, typename Task>
class get_result_aw_base
{
public:
    using value_type = T;

    get_result_aw_base(promise_wrapper<value_type> p)
        : m_promise{ ::std::move(p) }, 
          m_future{ m_promise.get_future() }
    {
    }

    bool await_ready() const noexcept 
    { 
/**************************************************************************************/
/***                          Critical Section Begin                                ***/
/**************************************************************************************/
/**/    this->lock_shared_state();                                                  /**/
/**/    toolpex_assert(m_lock);                                                     /**/
/**/    return m_future.ready(m_lock);                                              /**/
/**/}                                                                               /**/


/**/    //  This function will record the caller coroutine for resuming it.         /**/
/**/void await_suspend(task_on_the_fly h) noexcept                                  /**/
/**/{                                                                               /**/
/**/    // Dear developers:                                                         /**/
/**/    // This function should not throw (even potentially) anything.              /**/
/**/    // See also comments of `thread_pool::enqueue`                              /**/
/**/    toolpex_assert(this->m_lock);                                               /**/
/**/    auto lk = ::std::move(m_lock);                                              /**/
/**/    m_promise.set_caller(::std::move(h));                                       /**/
/**/    get_task_scheduler().enqueue(                                               /**/
/**/        static_cast<Task*>(this)->get_handler_to_schedule()                     /**/
/**/    );                                                                          /**/
/**************************************************************************************/
/***                        Critical Section End **2**                              ***/
/**************************************************************************************/
    }
    
    /*! \brief Get the ownership of the future type.
     *  \return the `koios::future<T>` object.
     */
    auto get_future() noexcept 
    { 
        return ::std::move(future()); 
    }

    auto& future() noexcept
    {
        // For which scheduled by user call 
        // `task::run()` or `task::run_and_get_future()` directly.
        toolpex_assert(!m_promise.caller_set());
        return m_future; 
    }

protected:
    // Only used in await_ready() !
    // The counterpart promise object should also acquire this lock.
    // In practice, the counterpart promise object of koios is `return_value_or_void`.
    void lock_shared_state() const noexcept { m_lock = m_future.get_shared_state_lock(); }

    // Only used in await_suspend and await_resume
    // to make the
    //                   ready => suspend
    //                          or
    //                   ready => resume 
    // opertions ATOMIC.
    //
    // XXX: Only call this function at 
    //      the end of `await_suspend` and `await_resume`
    //      (the end of critical section.)
    //
    // If you forget to call this function, 
    // thanks for RAII, the mutex will be unlocked, 
    // after the task's lifetime.
    //
    // XXX: If user just store the task object into a variable, 
    //      the destruction phase of `get_result_aw` will be delayed!.
    //      Thus, you had to call this function in 
    //      `await_resume` and `await_suspend` manually.
    //
    void unlock_shared_state() const noexcept 
    { 
        if (m_lock) m_lock.unlock(); 
    }

protected:
    promise_wrapper<value_type> m_promise;
    koios::future<T> m_future;
    mutable ::std::unique_lock<::std::mutex> m_lock{};
};

template<typename T, typename Task>
class get_result_aw
    : public get_result_aw_base<T, Task>
{
public:
    using value_type = T;
    using base = get_result_aw_base<T, Task>;
    using base::get_result_aw_base;
    
/**/decltype(auto) await_resume()                                                   /**/
/**/{                                                                               /**/
/**/    if (!this->m_lock) this->lock_shared_state();                               /**/
/**/    auto lk = ::std::move(this->m_lock);                                        /**/
/**/    return this->m_future.get_nonblk(lk);                                       /**/
/**************************************************************************************/
/***                        Critical Section End **2**                              ***/
/**************************************************************************************/
    }
};

template<typename Task>
class get_result_aw<void, Task>
    : public get_result_aw_base<void, Task>
{
public:
    using value_type = void;
    using base = get_result_aw_base<void, Task>;
    using base::get_result_aw_base;

/**/void await_resume()                                                             /**/
/**/{                                                                               /**/
/**/    if (!this->m_lock) this->lock_shared_state();                               /**/
/**/    auto lk = ::std::move(this->m_lock);                                        /**/
/**/    this->m_future.get_nonblk(lk);                                              /**/
/**************************************************************************************/
/***                        Critical Section End **3**                              ***/
/**************************************************************************************/
    }
};

KOIOS_NAMESPACE_END

#endif
