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
    op_peripheral_element(T&& t, ::std::pmr::polymorphic_allocator<::std::byte>& alloc)
        : m_buffer{ alloc.allocate(sizeof(T)) }
    {
        new (m_buffer) T(::std::forward<T>(t));
        m_deleter = +[](void* ptr) noexcept
        {
            T* p = reinterpret_cast<T*>(ptr);
            p->~T();
        };
    }

    op_peripheral_element(op_peripheral_element&& other) noexcept;
    op_peripheral_element& operator=(op_peripheral_element&& other) noexcept;
    ~op_peripheral_element() noexcept;
    ::std::byte* ptr() noexcept { return m_buffer; }

private:
    void delete_this() noexcept;

private:
    ::std::byte* m_buffer;
    void (*m_deleter) (void*) noexcept {};
};

class op_peripheral : public toolpex::move_only
{
public:
    op_peripheral(::std::pmr::memory_resource* mr = nullptr)
        : m_mr(mr ? mr : ::std::pmr::get_default_resource()),
          m_pa_perele(m_mr),
          m_pa_bytes(m_mr), 
          m_objs(m_pa_perele)
    {
    }

    template<typename DataT, typename... Args>
    requires (::std::is_nothrow_move_constructible_v<DataT>)
    DataT* add(Args&&... args)
    {
        auto* result = m_objs.emplace_back(DataT(::std::forward<Args>(args)...), m_pa_bytes).ptr();
        return reinterpret_cast<DataT*>(result);
    }

private:
    ::std::pmr::memory_resource* m_mr;
    ::std::pmr::polymorphic_allocator<op_peripheral_element> m_pa_perele;
    ::std::pmr::polymorphic_allocator<::std::byte> m_pa_bytes;
    ::std::list<op_peripheral_element, decltype(m_pa_perele)> m_objs;
};

} // namespace koios::uring

#endif
