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

#ifndef KOIOS_LOCAL_THREAD_SCHEDULER_H
#define KOIOS_LOCAL_THREAD_SCHEDULER_H

#include <coroutine>
#include <queue>

#include "koios/macros.h"
#include "koios/task_on_the_fly.h"

KOIOS_NAMESPACE_BEG

/*! \brief A tiny task scheduler which allow tasks run synchronously. */ 
class local_thread_scheduler
{
public:
    /*! This is sync call, which means only the scheduled task return, this function return.
     *  Of course, this will call the coroutine immediately.
     */
    void enqueue(task_on_the_fly h)
    {
        m_tasks.emplace(::std::move(h));
        this->run();
    }

private:
    void run() noexcept
    {
        while (!m_tasks.empty())
        {
            auto h = ::std::move(m_tasks.front());
            m_tasks.pop();
            try { h(); } catch (...) {}
        }
    }

private:
    // std::queue is not suppose to be there...
    ::std::queue<task_on_the_fly> m_tasks;
};

KOIOS_NAMESPACE_END

#endif
