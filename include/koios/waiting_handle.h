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

#ifndef KOIOS_WAITING_HANDLE_H
#define KOIOS_WAITING_HANDLE_H

#include "koios/per_consumer_attr.h"
#include "koios/task_on_the_fly.h"
#include "koios/runtime.h"

namespace koios
{

struct waiting_handle
{
    per_consumer_attr attr;
    task_on_the_fly task;
};

inline void wake_up(waiting_handle& h)
{
    auto t = ::std::move(h.task);
    [[assume(bool(t))]];
    get_task_scheduler().enqueue(h.attr, ::std::move(t));
}

void wake_up(task_on_the_fly f);

} // namespace koios

#endif
