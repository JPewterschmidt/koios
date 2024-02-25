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

#ifndef KOIOS_PER_CONSUMER_ATTR_H
#define KOIOS_PER_CONSUMER_ATTR_H

#include <thread>

#include "koios/macros.h"

#undef BLOCK_SIZE
#include "concurrentqueue/concurrentqueue.h"

KOIOS_NAMESPACE_BEG

struct per_consumer_attr
{
    ::std::thread::id thread_id{ ::std::this_thread::get_id() };
    ::std::thread::id main_thread_id;
    size_t number_of_threads{};
    moodycamel::ConsumerToken* q_producer_token{};
};

KOIOS_NAMESPACE_END

#endif
