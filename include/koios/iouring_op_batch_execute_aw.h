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

#ifndef KOIOS_IOURING_OP_BATCH_EXECUTE_AW_H
#define KOIOS_IOURING_OP_BATCH_EXECUTE_AW_H

#include <vector>
#include "koios/task_on_the_fly.h"
#include "koios/iouring_ioret.h"

namespace koios::uring
{

class op_batch_rep;
class op_batch_execute_aw
{
public:
    op_batch_execute_aw(op_batch_rep& rep) noexcept
        : m_rep{ &rep }
    {
    }

    constexpr bool await_ready() const noexcept { return false; }
    void await_suspend(task_on_the_fly t);
    constexpr void await_resume() const noexcept {}

private:
    op_batch_rep* m_rep;      
};

} // namespace koios::uring

#endif
