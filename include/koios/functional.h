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

#ifndef KOIOS_FUNCTIONAL_H
#define KOIOS_FUNCTIONAL_H

#include "koios/macros.h"
#include "koios/task.h"
#include "koios/task_concepts.h"
#include <concepts>
#include <tuple>

KOIOS_NAMESPACE_BEG

auto identity(auto arg) -> task<decltype(arg)>
{
    co_return arg;
}

template<typename Func, typename... Args>
requires (
    task_callable_concept<Func> 
    and !emitter_task_callable_concept<Func>)
auto make_emitter(Func&& f, Args&&... args) 
    -> emitter_task<typename toolpex::get_return_type_t<Func>::value_type>
{
    co_return co_await ::std::forward<Func>(f)(::std::forward<Args>(args)...);
}

template<typename Func, typename... Args>
requires (emitter_task_callable_concept<Func>)
auto make_emitter(Func&& f, Args&&... args)
    -> emitter_task<typename toolpex::get_return_type_t<Func>::value_type>
{
    return ::std::forward<Func>(f)(::std::forward<Args>(args)...);
}

template<awaitible_concept... Aws>
auto when_all(Aws&&... aws)
    -> task<::std::tuple<awaitable_result_type_t<Aws>...>>
{
    co_return ::std::make_tuple((co_await aws)...);
}

KOIOS_NAMESPACE_END

#endif
