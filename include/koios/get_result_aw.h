#ifndef KOIOS_GET_RESULT_AW_H
#define KOIOS_GET_RESULT_AW_H

#include <future>
#include <memory>
#include <cassert>

#include "koios/macros.h"
#include "koios/local_thread_scheduler.h"
#include "koios/task_scheduler_wrapper.h"
#include "koios/task_on_the_fly.h"

KOIOS_NAMESPACE_BEG

/*! \brief Awaitable base class for coroutine call in another coroutine task.
 *  
 *  Contains the requried member function which needed by the compiler,
 *  also holds the ownership of future.
 *
 *  \tparam T result type of the current task.
 *  \tparam Task `task` type of the current task.
 *  \tparam DriverPolicy Decide where to resume the caller task and the task itself.
 */
template<typename T, typename Task, typename DriverPolicy>
class get_result_aw_base
{
public:
    using value_type = T;

    get_result_aw_base(promise_wrapper<value_type> p)
        : m_promise{ ::std::move(p) }, 
          m_future{ m_promise.get_future() }
    {
    }

    constexpr bool await_ready() const noexcept { return false; }

    /*! This function will record the caller coroutine for resuming it.
     *  It will also schedule this current task with `DriverPolicy`.
     */
    void await_suspend(task_on_the_fly h) noexcept
    {
        // Dear developers: 
        // This function should not throw (even potentially) anything.
        // See also comments of `thread_pool::enqueue`

        m_promise.set_caller(::std::move(h));
        DriverPolicy{}.scheduler().enqueue(
            static_cast<Task*>(this)->get_handler_to_schedule()
        );
    }
    
    /*! \brief Get the ownership of the future type.
     *  \return the `std::future<T>` object.
     */
    auto get_future() noexcept 
    { 
        // For which scheduled by user call `task::run()` or `task::run_and_get_future()` directly.
        assert(!m_promise.caller_set());
        return ::std::move(m_future); 
    }

protected:
    promise_wrapper<value_type> m_promise;
    ::std::future<T> m_future;
};

template<typename T, typename Task, typename DriverPolicy>
class get_result_aw : public get_result_aw_base<T, Task, DriverPolicy>
{
public:
    using value_type = T;

    get_result_aw(promise_wrapper<value_type> promise)
        : get_result_aw_base<T, Task, DriverPolicy>{ ::std::move(promise) }
    {
    }
    
    decltype(auto) await_resume() { return this->m_future.get(); }
};

template<typename Task, typename DriverPolicy>
class get_result_aw<void, Task, DriverPolicy> : public get_result_aw_base<void, Task, DriverPolicy>
{
public:
    get_result_aw(promise_wrapper<void> promise)
        : get_result_aw_base<void, Task, DriverPolicy>{ ::std::move(promise) }
    {
    }

    void await_resume() { this->m_future.get(); }
};

KOIOS_NAMESPACE_END

#endif
