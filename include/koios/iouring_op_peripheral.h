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

#ifndef KOIOS_IOURING_OP_PERIPHERAL_H
#define KOIOS_IOURING_OP_PERIPHERAL_H

#include <vector>
#include <concepts>
#include <memory>
#include <type_traits>
#include "toolpex/move_only.h"

namespace koios::uring
{

class op_peripheral_element
{
public:
    template<typename T>
    op_peripheral_element(T&& t)
        : m_buffer{ new ::std::byte[sizeof(T)] }
    {
        new (m_buffer.get()) T(::std::forward<T>(t));
        m_deleter = +[](void* ptr) noexcept
        {
            T* p = reinterpret_cast<T*>(ptr);
            p->~T();
        };
    }

    op_peripheral_element(op_peripheral_element&& other) noexcept;
    op_peripheral_element& operator=(op_peripheral_element&& other) noexcept;
    ~op_peripheral_element() noexcept;
    ::std::byte* ptr() noexcept { return m_buffer.get(); }

private:
    void delete_this() noexcept;

private:
    ::std::unique_ptr<::std::byte[]> m_buffer;
    void (*m_deleter) (void*) noexcept {};
};

class op_peripheral : public toolpex::move_only
{
public:
    constexpr op_peripheral() noexcept = default;

    template<typename DataT, typename... Args>
    requires (::std::is_nothrow_move_constructible_v<DataT>)
    DataT* add(Args&&... args)
    {
        auto elem = op_peripheral_element(DataT(::std::forward<Args>(args)...));
        auto* result = m_peripheral_datas.emplace_back(::std::move(elem)).ptr();
        return reinterpret_cast<DataT*>(result);
    }

private:
    ::std::vector<op_peripheral_element> m_peripheral_datas;
};

} // namespace koios::uring

#endif
