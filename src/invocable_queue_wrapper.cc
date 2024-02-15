/* koios, a c++ async runtime library.
 * copyright (c) 2024  jeremy pewterschmidt
 * 
 * this program is free software; you can redistribute it and/or
 * modify it under the terms of the gnu general public license
 * as published by the free software foundation; either version 2
 * of the license, or (at your option) any later version.
 * 
 * this program is distributed in the hope that it will be useful,
 * but without any warranty; without even the implied warranty of
 * merchantability or fitness for a particular purpose.  see the
 * gnu general public license for more details.
 * 
 * you should have received a copy of the gnu general public license
 * along with this program; if not, write to the free software
 * foundation, inc., 51 franklin street, fifth floor, boston, ma  02110-1301, usa.
 */

#include <utility>

#include "koios/invocable_queue_wrapper.h"

KOIOS_NAMESPACE_BEG

invocable_queue_wrapper::
~invocable_queue_wrapper() noexcept
{
    if (m_dtor) m_dtor(m_storage.get());
}

invocable_queue_wrapper::
invocable_queue_wrapper(invocable_queue_wrapper&& other) noexcept
    : m_storage     { ::std::move(other.m_storage)              }, 
      m_dtor        { ::std::exchange(other.m_dtor, nullptr)    }, 
      m_empty_impl  { other.m_empty_impl                        },
      m_enqueue_impl{ other.m_enqueue_impl                      }, 
      m_dequeue_impl{ other.m_dequeue_impl                      }, 
      m_size_impl   { other.m_size_impl                         }, 
      m_thread_specific_preparation{ other.m_thread_specific_preparation }
{
}

KOIOS_NAMESPACE_END
