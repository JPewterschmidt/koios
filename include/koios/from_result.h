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

#ifndef KOIOS_FROM_RESULT_H
#define KOIOS_FROM_RESULT_H

#include <coroutine>
#include <utility>

#include "koios/macros.h"
#include "koios/task.h"

KOIOS_NAMESPACE_BEG

namespace from_result_detial
{
    template<typename Ret>
    class from_result_aw
    {
    public: 
        constexpr from_result_aw(Ret r)
            : m_obj{ ::std::move(r) }
        {
        }

        constexpr bool await_ready() const noexcept { return true; }
        constexpr void await_suspend(::std::coroutine_handle<>) const noexcept {}
        constexpr Ret await_resume() noexcept { return ::std::move(m_obj); }

    private:
        Ret m_obj;
    };
}

template<typename T>
auto from_result(T&& r)
{
    return from_result_detial::from_result_aw(::std::forward<T>(r));
}

KOIOS_NAMESPACE_END

#endif
