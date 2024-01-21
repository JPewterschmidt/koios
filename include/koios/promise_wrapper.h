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

#ifndef KOIOS_PROMISE_WRAPPER_H
#define KOIOS_PROMISE_WRAPPER_H

#include <coroutine>
#include <future>
#include <memory>

#include "koios/macros.h"
#include "koios/task_on_the_fly.h"
#include "koios/future.h"

KOIOS_NAMESPACE_BEG

/*! \brief Type Erasure class of promise. */
template<typename T>
class promise_wrapper
{
public:
    template<typename P>
    promise_wrapper(P& p) noexcept
        : m_promise{ ::std::addressof(p) }, 
          m_set_caller_impl{ +[](void* p, task_on_the_fly h) { static_cast<P*>(p)->set_caller(::std::move(h)); } }, 
          m_get_future_impl{ +[](void* p) { return static_cast<P*>(p)->get_future(); } }
    {
    }

    void set_caller(task_on_the_fly h) 
    {
        m_set_caller_impl(m_promise, ::std::move(h));
        m_caller_set = true;
    }
    
    koios::future<T> get_future()
    {
        return m_get_future_impl(m_promise);
    }

    bool caller_set() const noexcept { return m_caller_set; }

private:
    void* const m_promise;
    void (* const m_set_caller_impl)(void*, task_on_the_fly);
    koios::future<T> (* const m_get_future_impl)(void*);
    bool m_caller_set{};
};

KOIOS_NAMESPACE_END

#endif
