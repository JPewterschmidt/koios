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

#ifndef KOIOS_DRIVER_POLICY_H
#define KOIOS_DRIVER_POLICY_H

#include "koios/macros.h"
#include "koios/task_scheduler_concept.h"
#include "koios/local_thread_scheduler.h"
#include "koios/runtime.h"
#include "koios/task_scheduler_wrapper.h"

KOIOS_NAMESPACE_BEG

template<typename DP>
concept driver_policy_concept = requires(DP dp)
{
    { dp.scheduler() } -> task_scheduler_concept;
};

/*! \brief One of the drive policy, make the `task` runs asynchronous. */
struct run_this_async
{
    /*! \return A global asynchronous task scheduler. */
    task_scheduler_wrapper scheduler() noexcept
    {
        return { get_task_scheduler() };
    };
};

/*! \brief One of the drive policy, make the `task` runs synchronous. */
struct run_this_sync
{
    /*! \return A local synchronous task scheduler. */
    task_scheduler_owned_wrapper scheduler()
    {
        return { local_thread_scheduler{} };
    };
};

KOIOS_NAMESPACE_END

#endif
