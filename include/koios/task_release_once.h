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

#ifndef KOIOS_TASK_RELEASE_ONCE_H
#define KOIOS_TASK_RELEASE_ONCE_H

#include "koios/macros.h"
#include "koios/task_on_the_fly.h"
#include <memory>
#include <atomic>
#include <optional>

KOIOS_NAMESPACE_BEG

class task_release_once
{
public:
    task_release_once(task_on_the_fly t)
        : m_task{ ::std::move(t) }
    {
    }

    ::std::optional<task_on_the_fly> release()
    {
        bool expected{ true };
        if (m_valid.compare_exchange_strong(expected, false))
        {
            return ::std::move(m_task);
        }
        return {};
    }
    
private:
    ::std::atomic_bool m_valid{true};
    task_on_the_fly m_task;
};

KOIOS_NAMESPACE_END

#endif
