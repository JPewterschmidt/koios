// This file is part of Koios
// https://github.com/JPewterschmidt/koios
//
// Copyleft 2023 - 2024, ShiXin Wang. All wrongs reserved.

#ifndef KOIOS_ASYNC_AWAITING_HUB_H
#define KOIOS_ASYNC_AWAITING_HUB_H

#include "koios/macros.h"
#include "koios/task_on_the_fly.h"
#include "koios/runtime.h"
#undef BLOCK_SIZE
#include "concurrentqueue/concurrentqueue.h"

KOIOS_NAMESPACE_BEG

/*! \brief A general async resource dispatcher queue
 *
 *  This class allows you to implements your async resource dispatcher queue,
 *  will clearly it's based on a concurrent queue, namely `moodycamel::ConcurrentQueue`.
 *  This queue will store those task handler (`task_on_the_fly`) 
 *  which want to get the specific resource asynchronously.
 *  For example, the coroutine mutex could be implemented by deriving from this class
 *  (bu the actual practice may not,)
 *
 *  Those classes derived from this are typically awaitable object class.
 *  More detials see those doc on each member functions.
 */
class async_awaiting_hub
{
public:
    /*! \brief add a task (handler) to the (a)waiting queue.
     *  \param y the task handler
     *
     *  This function should be called by `await_suspend` of the derived class.
     */
    void add_awaiting(task_on_the_fly y) noexcept
    {
        m_awaitings.enqueue(::std::move(y));
    }

    /*! \brief Wake up the task on the front of the waiting queue.
     *
     *  user should implement a guard which utilizes the RAII feature of C++
     *  to call this function in the destructor of that RAII guard, 
     *  to wake up the next waiting task. 
     *  
     *  \retval true Successfully wake up a awaiting task.
     *  \retval false there's no task awaiting.
     */
    bool may_wake_next() noexcept
    {
        task_on_the_fly f{};
        if (!m_awaitings.try_dequeue(f))
            return false;
        [[assume(bool(f))]];
        get_task_scheduler().enqueue(::std::move(f));       
        return true;
    }

    /*! \brief Wake up all the tasks in the waiting queue.
     *
     *  user should implement a guard which utilizes the RAII feature of C++
     *  to call this function in the destructor of that RAII guard, 
     *  to wake up the next waiting task. 
     *
     *  \retval true Successfully wake up a awaiting task.
     *  \retval false there's no task awaiting.
     */
    bool may_wake_all() noexcept
    {
        task_on_the_fly f{};
        auto& schr = get_task_scheduler();
        bool result{};
        while (m_awaitings.try_dequeue(f))
        {
            result = true;
            [[assume(bool(f))]];
            schr.enqueue(::std::move(f));       
        }
        return result;
    }
    
private:
    moodycamel::ConcurrentQueue<task_on_the_fly> m_awaitings;
};

KOIOS_NAMESPACE_END

#endif
