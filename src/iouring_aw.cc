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

#include "koios/iouring_aw.h"
#include "koios/runtime.h"

KOIOS_NAMESPACE_BEG

namespace uring { 

iouring_aw::iouring_aw(::io_uring_sqe sqe) 
    : m_ret{ ::std::make_shared<ioret>() }, 
      m_sqe{ sqe }
{
}

void iouring_aw::
await_suspend(task_on_the_fly h) 
{
    koios::get_task_scheduler().add_event<iouring_event_loop>(
        ::std::move(h), m_ret, m_sqe
    );
}

} // namespace uring

KOIOS_NAMESPACE_END
