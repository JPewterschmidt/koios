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

#ifndef TASK_PROMISE_BASE_H
#define TASK_PROMISE_BASE_H

#include "koios/macros.h"
#include "koios/local_thread_scheduler.h"
#include "koios/runtime.h"
#include "koios/task_scheduler_wrapper.h"
#include "koios/task_on_the_fly.h"
#include "koios/exceptions.h" // logging
#include <sstream>            // logging
 
KOIOS_NAMESPACE_BEG

/*! \brief Coroutine which has been suspended by this awaitable class will be `destroy()`.
 *  Often used as awaitable object for `final_suspend()`.
 *  Will take ownership of the task's handler at the end of the `task` lifetime and `destroy()` it. 
 *  There are two more classes that can hold handler ownership.
 *  
 *  \see `task`
 *  \see `task_on_the_fly`
 */
class destroy_aw 
{
public:
    constexpr bool await_ready() const noexcept { return false; }
    constexpr void await_suspend(task_on_the_fly h) const noexcept { }
    constexpr void await_resume() const noexcept { }
};

/*! \brief life-cycle control class of a task
 *  \tparam FinalSuspendAwaitable Class which contains the methods
 *                                when coroutine get into the final suspend phase.
 */
template<typename FinalSuspendAwaitable = destroy_aw>
class promise_base
{
public:
    constexpr ::std::suspend_always initial_suspend() const noexcept
        { return {}; }
    constexpr FinalSuspendAwaitable final_suspend() const noexcept
        { return {}; }
};

KOIOS_NAMESPACE_END

#endif
