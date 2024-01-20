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

#ifndef KOIOS_TASK_CONCEPTS_H
#define KOIOS_TASK_CONCEPTS_H

#include <concepts>
#include <coroutine>

#include "koios/macros.h"
#include "toolpex/concepts_and_traits.h"
#include "koios/task_on_the_fly.h"
#include "koios/future.h"

KOIOS_NAMESPACE_BEG

template<typename Aw>
concept awaitible_concept = requires(Aw a)
{
    { a.await_ready() } -> ::std::same_as<bool>;
    { a.await_suspend(::std::declval<::std::coroutine_handle<>>()) };
};

template<typename T>
concept task_concept = requires(T t)
{
    typename T::value_type;
    { static_cast<task_on_the_fly>(t) };
    { t() };
    { t.run_and_get_future() } -> toolpex::is_specialization_of<::koios::future>;
} and awaitible_concept<T>;

template<typename Func>
concept task_callable_concept = task_concept<toolpex::get_return_type_t<Func>>;

KOIOS_NAMESPACE_END

#endif
