/* Koios, A c++ async runtime library.
 * Copyright (C) 2023  Jeremy Pewterschmidt
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
     */
    void may_wake_next() noexcept
    {
        task_on_the_fly f{};
        if (!m_awaitings.try_dequeue(f))
            return;
        [[assume(bool(f))]];
        get_task_scheduler().enqueue(::std::move(f));       
    }
    
private:
    moodycamel::ConcurrentQueue<task_on_the_fly> m_awaitings;
};

KOIOS_NAMESPACE_END

#endif
