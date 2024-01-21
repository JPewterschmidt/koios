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

#ifndef KOIOS_THIS_TASK_H
#define KOIOS_THIS_TASK_H

#include "macros.h"
#include "runtime.h"
#include <chrono>

KOIOS_NAMESPACE_BEG

namespace this_task
{
    template<typename Duration>
    class sleep_await
    {
    public:
        sleep_await(Duration dura)
            : m_dura{ dura }
        {
        }

        bool await_ready() const noexcept
        {
            return m_dura == Duration{};
        }

        void await_suspend(task_on_the_fly h) const 
        {
            get_task_scheduler().add_event<timer_event_loop>(m_dura, ::std::move(h));
        }

        constexpr void await_resume() const noexcept { }
        
    private:
        Duration m_dura;
    };

    auto sleep_for(auto dura)
    {
        return sleep_await{ dura };
    }
}

KOIOS_NAMESPACE_END

#endif
