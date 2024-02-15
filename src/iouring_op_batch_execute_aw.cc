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

#include "koios/iouring_op_batch_execute_aw.h"
#include "koios/iouring_op_batch.h"
#include "koios/runtime.h"
#include "toolpex/bit_mask.h"

namespace koios::uring
{

void op_batch_execute_aw::
await_suspend(task_on_the_fly t)
{
    m_rep->set_user_data(t.address());
    auto& last_sqe = m_rep->back();
    last_sqe.flags = toolpex::bit_mask{last_sqe.flags}
       .remove(static_cast<uint8_t>(IOSQE_IO_LINK));

    get_task_scheduler().add_event<iouring_event_loop>(
        ::std::move(t), 
        *m_rep
    );
}

} // namespace koios::uring
