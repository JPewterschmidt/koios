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

#ifndef KOIOS_EXPECTED_CONCEPTS_H
#define KOIOS_EXPECTED_CONCEPTS_H

#include "koios/macros.h"
#include "toolpex/concepts_and_traits.h"
#include "koios/task_concepts.h"

KOIOS_NAMESPACE_BEG

template<typename Exp>
concept regular_expected_like_concept = requires (Exp e)
{
    e.error();
    { e.has_value() } -> ::std::same_as<bool>;
};

template<typename Exp>
concept expected_like_astask_concept = 
    regular_expected_like_concept<typename Exp::value_type>;

template<typename Exp>
concept expected_like_concept = 
    regular_expected_like_concept<Exp> or expected_like_astask_concept<Exp>;

template<typename ExpFunc>
concept expected_callable_concept = expected_like_concept< 
    toolpex::get_return_type_t<ExpFunc> 
>;

KOIOS_NAMESPACE_END

#endif
