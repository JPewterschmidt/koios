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

#ifndef KOIOS_MOODYCAMEL_QUEUE_WRAPPER_H
#define KOIOS_MOODYCAMEL_QUEUE_WRAPPER_H

#include <functional>
#include <optional>

#include "koios/macros.h"
#include "koios/per_consumer_attr.h"

#undef BLOCK_SIZE
#include "concurrentqueue/concurrentqueue.h"

KOIOS_NAMESPACE_BEG

/*! \brief The moodycamel Concurrent queue wrapper. 
 *         To satisfy the `invocable_queue_wrapper`.
 *  This is a thread-safe class.
 */
class moodycamel_queue_wrapper
{
public:
    using invocable_type = ::std::move_only_function<void()>;
    using queue_type = moodycamel::ConcurrentQueue<invocable_type>;

public:
    void enqueue(invocable_type&& func);
    ::std::optional<invocable_type> dequeue();
    bool empty() const;
    size_t size() const noexcept { return m_q.size_approx(); }

private:
    queue_type m_q;
};

KOIOS_NAMESPACE_END

#endif
