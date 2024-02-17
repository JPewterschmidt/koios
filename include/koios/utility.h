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

#ifndef KOIOS_UTILITY_H
#define KOIOS_UTILITY_H

#include "koios/macros.h"
#include "koios/functional.h"

KOIOS_NAMESPACE_BEG

inline auto from_result(auto&& r)
{
    return identity(::std::forward<decltype(r)>(r));
}

inline consteval bool is_profiling_mode()
{
#ifdef KOIOS_PROFILING
    return true;
#else
    return false;
#endif
}

KOIOS_NAMESPACE_END

#endif
