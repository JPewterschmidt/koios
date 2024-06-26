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

#ifndef KOIOS_TASK_SCHEDULER_CONCEPT_H
#define KOIOS_TASK_SCHEDULER_CONCEPT_H

#include <concepts>
#include <coroutine>
#include <type_traits>

#include "koios/macros.h"
#include "koios/task_on_the_fly.h"

KOIOS_NAMESPACE_BEG

template<typename TS>
concept task_scheduler_concept = requires(TS ts)
{
    //{ ts.enqueue(::std::declval<::std::coroutine_handle<>>()) };
    { ts.enqueue(::std::declval<koios::task_on_the_fly>()) };
};

template<typename TS>
concept task_scheduler_wrapper_concept = task_scheduler_concept<TS> && requires(TS ts)
{
    { typename TS::is_wrapper_t{} } -> ::std::same_as<::std::true_type>;
};

KOIOS_NAMESPACE_END

#endif
