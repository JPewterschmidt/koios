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

#include "koios/moodycamel_queue_wrapper.h"

KOIOS_NAMESPACE_BEG

void 
moodycamel_queue_wrapper::
enqueue(invocable_type&& func)
{
    m_q.enqueue(::std::move(func));
}

::std::optional<moodycamel_queue_wrapper::invocable_type> 
moodycamel_queue_wrapper::
dequeue()
{
    invocable_type func;
    if (!m_q.try_dequeue(func))
        return {};
    return func;
}

bool
moodycamel_queue_wrapper::
empty() const
{
    return m_q.size_approx() == 0;
}

KOIOS_NAMESPACE_END
