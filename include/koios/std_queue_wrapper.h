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

#ifndef KOIOS_STD_QUEUE_WRAPPER_H
#define KOIOS_STD_QUEUE_WRAPPER_H

#include <queue>
#include <mutex>
#include "koios/macros.h"
#include "toolpex/spin_lock.h"

KOIOS_NAMESPACE_BEG

/*! \brief Wrapper class of `std::queue<::std::move_only_function<void>>`. 
 *         To satisfy the `invocable_queue_wrapper`. 
 *
 *  This is a thread-safe class.
 */
class std_queue_wrapper
{
public:
    using invocable_type = ::std::move_only_function<void()>;
    using queue_type = ::std::queue<invocable_type>;

    std_queue_wrapper() = default;

    void enqueue(invocable_type&& func)
    {
        ::std::unique_lock lk{ m_lock };
        m_q.emplace(::std::move(func));
    }

    ::std::optional<invocable_type> dequeue()
    {
        ::std::unique_lock lk{ m_lock };
        if (m_q.empty()) return {};
        auto result = ::std::move(m_q.front());
        m_q.pop();
        return { ::std::move(result) };
    }

    bool empty() const noexcept 
    { 
        ::std::unique_lock lk{ m_lock };
        return m_q.empty(); 
    }

    size_t size() const noexcept 
    { 
        ::std::unique_lock lk{ m_lock };
        return m_q.size(); 
    }

    std_queue_wrapper(std_queue_wrapper&& other) noexcept
        : m_q{ ::std::move(other.m_q) }
    {
    }

private:
    queue_type m_q;   
    mutable ::std::mutex m_lock;
};

KOIOS_NAMESPACE_END

#endif
