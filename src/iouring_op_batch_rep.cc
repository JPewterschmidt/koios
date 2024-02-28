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

#include "koios/iouring_op_batch_rep.h"
#include <iterator>

namespace koios::uring
{

::io_uring_sqe* op_batch_rep::get_sqe()
{
    if (!was_timeout_set())
        return &m_sqes.emplace_back();
    return &(*m_sqes.emplace(prev(m_sqes.end())));
}

void op_batch_rep::set_user_data(void* userdata)
{
    for (auto& sqe : m_sqes)
        ::io_uring_sqe_set_data(&sqe, userdata);
}

void op_batch_rep::set_user_data(uint64_t userdata)
{
    for (auto& sqe : m_sqes)
        ::io_uring_sqe_set_data64(&sqe, userdata);
}

}
