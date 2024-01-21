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

#ifndef KOIOS_TASK_SCHEDULER_H
#define KOIOS_TASK_SCHEDULER_H

#include <coroutine>
#include <concepts>

#include "koios/macros.h"
#include "koios/task_concepts.h"
#include "koios/thread_pool.h"
#include "koios/task_on_the_fly.h"
#include "koios/std_queue_wrapper.h"
#include "koios/moodycamel_queue_wrapper.h"
#include "koios/work_stealing_queue.h"

KOIOS_NAMESPACE_BEG

/*! \brief the async task scheduler class. */
class task_scheduler : public thread_pool
{
public:
    using queue_type = work_stealing_queue<moodycamel_queue_wrapper>;

public:
    /*! \param thr_cnt the number of thread you want.
     *  \param manually_stop_type a tag which indecate 
     *                            your willing of how the destructor behave. Just like `thread_pool`.
     */
    explicit task_scheduler(size_t thr_cnt, manually_stop_type)
        : thread_pool{ thr_cnt, queue_type{}, manually_stop }
    {
    }

    explicit task_scheduler(size_t thr_cnt)
        : thread_pool{ thr_cnt, queue_type{} }
    {
    }

    void enqueue(task_concept auto t) noexcept
    {
        enqueue(t.move_out_coro_handle());
    }

    void enqueue(task_on_the_fly h) noexcept
    {
        if (h) thread_pool::enqueue_no_future([h = ::std::move(h)] mutable { h(); });
    }

    void enqueue(const per_consumer_attr& ca, task_concept auto t)
    {
        enqueue(ca, task_on_the_fly(t));
    }

    void enqueue(const per_consumer_attr& ca, task_on_the_fly h)
    {
        if (h) thread_pool::enqueue_no_future(ca, [h = ::std::move(h)] mutable { h(); });
    }

    virtual void stop() noexcept { thread_pool::stop(); }
    void quick_stop() noexcept { thread_pool::quick_stop(); }

    virtual ~task_scheduler() noexcept {}
};

KOIOS_NAMESPACE_END

#endif
