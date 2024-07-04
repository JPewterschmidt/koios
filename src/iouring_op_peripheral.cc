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

#include "toolpex/assert.h"
#include "koios/iouring_op_peripheral.h"

namespace koios::uring
{

op_peripheral_element::
op_peripheral_element(op_peripheral_element&& other) noexcept
    : m_buffer{ ::std::move(other.m_buffer) }, 
      m_deleter{ other.m_deleter }
{
    other.m_buffer = nullptr;
}

op_peripheral_element& 
op_peripheral_element::
operator=(op_peripheral_element&& other) noexcept
{
    this->delete_this();
    m_buffer = ::std::move(other.m_buffer);
    m_deleter = other.m_deleter;
    other.m_buffer = nullptr;
    return *this;
}

op_peripheral_element::
~op_peripheral_element() noexcept
{
    this->delete_this();
}

void 
op_peripheral_element::
delete_this() noexcept
{
    toolpex_assert(m_deleter);
    if (m_buffer) m_deleter(m_buffer.get());
}

} // namespace koios::uring
