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

#pragma once
#ifndef KOIOS_RUNTIME_H
#define KOIOS_RUNTIME_H

#include "koios/macros.h"
#include "koios/task_scheduler.h"
#include "koios/task_scheduler_concept.h"
#include "koios/event_loop.h"
#include "koios/timer.h"
#include "koios/iouring.h"

#include <memory>
#include <source_location>

KOIOS_NAMESPACE_BEG

using event_loop_t = event_loop<
    timer_event_loop, 
    iouring_event_loop
>;

::std::unique_ptr<task_scheduler>& 
get_task_scheduler_ptr(::std::source_location sl);

inline 
task_scheduler_concept 
auto& get_task_scheduler(
    ::std::source_location sl = ::std::source_location::current())
{
    return static_cast<event_loop_t&>(
        *get_task_scheduler_ptr(::std::move(sl))
    );
}

void runtime_init(size_t numthr);
void runtime_init(size_t numthr, manually_stop_type);
int runtime_exit();

void runtime_reload(size_t numthr);
void runtime_reload(size_t numthr, manually_stop_type);

::std::unique_ptr<task_scheduler> 
exchange_task_scheduler(::std::unique_ptr<task_scheduler> other);

KOIOS_NAMESPACE_END

#endif
