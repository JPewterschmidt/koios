// This file is part of Koios
// https://github.com/JPewterschmidt/koios
//
// Copyleft 2023 - 2024, ShiXin Wang. All wrongs reserved.

#ifndef KOIOS_IOURING_OP_PERIPHERAL_H
#define KOIOS_IOURING_OP_PERIPHERAL_H

#include <list>
#include <utility>
#include <concepts>
#include <memory>
#include <memory_resource>
#include <type_traits>
#include "toolpex/move_only.h"

namespace koios::uring
{

class op_peripheral_element
{
public:
    template<typename T>
    op_peripheral_element(T&& t, const ::std::pmr::polymorphic_allocator<::std::byte>& alloc)
        : m_alloc{ alloc }, m_buffer{ m_alloc.allocate(sizeof(T)) }
    {
        new (m_buffer) T(::std::forward<T>(t));
        m_deleter = +[](void* ptr) noexcept -> ::std::size_t
        {
            T* p = reinterpret_cast<T*>(ptr);
            p->~T();
            return sizeof(T);
        };
    }

    op_peripheral_element(op_peripheral_element&& other) noexcept;
    op_peripheral_element& operator=(op_peripheral_element&& other) noexcept;
    ~op_peripheral_element() noexcept;
    ::std::byte* ptr() noexcept { return m_buffer; }

private:
    ::std::size_t delete_this() noexcept;

private:
    ::std::pmr::polymorphic_allocator<::std::byte> m_alloc;
    ::std::byte* m_buffer;
    ::std::size_t (*m_deleter) (void*) noexcept {};
};

class op_peripheral : public toolpex::move_only
{
public:
    op_peripheral(::std::pmr::memory_resource* mr)
        : m_mr{ mr }, 
          m_objs(::std::pmr::polymorphic_allocator<op_peripheral_element>(m_mr))
    {
    }

    op_peripheral(op_peripheral&&) noexcept = default;
    op_peripheral& operator=(op_peripheral&&) noexcept = default;

    template<typename DataT, typename... Args>
    requires (::std::is_nothrow_move_constructible_v<DataT>)
    DataT* add(Args&&... args)
    {
        auto* result = m_objs.emplace_back(
            DataT(::std::forward<Args>(args)...), 
            ::std::pmr::polymorphic_allocator<::std::byte>(m_mr)
        ).ptr();
        return reinterpret_cast<DataT*>(result);
    }

private:
    ::std::pmr::memory_resource* m_mr;
    ::std::list<op_peripheral_element, ::std::pmr::polymorphic_allocator<op_peripheral_element>> m_objs;
};

} // namespace koios::uring

#endif
