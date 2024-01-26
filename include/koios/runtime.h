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
#include "koios/user_event_loops.h"

#include <memory>
#include <source_location>

KOIOS_NAMESPACE_BEG

using event_loop_t = event_loop<
    timer_event_loop, 
    iouring_event_loop, 
    user_event_loops
>;

::std::unique_ptr<event_loop_t>& 
get_task_scheduler_ptr(::std::source_location sl);

inline 
task_scheduler_concept 
auto& get_task_scheduler(
    ::std::source_location sl = ::std::source_location::current())
{
    return *get_task_scheduler_ptr(::std::move(sl));
}

class runtime_init_config
{
public:
    constexpr runtime_init_config() noexcept = default;
    constexpr runtime_init_config(size_t numthr) noexcept
        : m_number_thread{ numthr }
    {
    }

    auto&& number_thread(size_t num) noexcept { m_number_thread = num; return *this; }
    auto&& set_manually_stop(bool val = true) noexcept { m_manualy_stop = val; return *this; }
    auto&& add_user_event_loop(user_event_loop_interface::sptr loop)
    { 
        m_loops.emplace_back(::std::move(loop));
        return *this; 
    }

    friend void runtime_init(runtime_init_config cfg);
    
private:
    size_t m_number_thread{12};
    ::std::vector<user_event_loop_interface::sptr> m_loops;
    bool m_manualy_stop{};
};

void runtime_init(runtime_init_config cfg);
void runtime_reload(runtime_init_config cfg);
int runtime_exit();

::std::unique_ptr<task_scheduler> 
exchange_task_scheduler(::std::unique_ptr<task_scheduler> other);

KOIOS_NAMESPACE_END

#endif
