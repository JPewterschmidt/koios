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

#ifndef KOIOS_GET_HANDLE_AW_H
#define KOIOS_GET_HANDLE_AW_H

#include "koios/macros.h"
#include "koios/task_on_the_fly.h"
#include <cstddef>
#include <cstdint>
#include <functional>

KOIOS_NAMESPACE_BEG

using task_id = void*;

/*! \brief Awaitable class which get the coroutine task id (handler address). */
class get_id_aw
{
public:
    /*! \brief  Always return `false`. */
    constexpr bool await_ready() const noexcept { return false; }
    void await_suspend(task_on_the_fly h) noexcept;
    task_id await_resume() const noexcept { return m_result; }

private:
    task_id m_result{};
};

KOIOS_NAMESPACE_END

#endif
