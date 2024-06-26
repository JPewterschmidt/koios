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

#ifndef KOIOS_EVENT_LOOP_CONCEPTS_H
#define KOIOS_EVENT_LOOP_CONCEPTS_H

#include "koios/macros.h"
#include "koios/per_consumer_attr.h"
#include "toolpex/is_specialization_of.h"
#include <concepts>
#include <utility>
#include <chrono>

KOIOS_NAMESPACE_BEG

template<typename EL>
concept event_loop_concept = requires(EL e)
{
    { &EL::thread_specific_preparation };
    e.do_occured_nonblk();
    //{ &EL::add_event };
    e.stop();
    e.quick_stop();
    e.until_done();
    { e.max_sleep_duration(::std::declval<per_consumer_attr>()) } 
        -> toolpex::is_specialization_of<::std::chrono::duration>;
} && ::std::default_initializable<EL>;

KOIOS_NAMESPACE_END

#endif
