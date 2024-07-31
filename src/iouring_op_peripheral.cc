// This file is part of Koios
// https://github.com/JPewterschmidt/koios
//
// Copyleft 2023 - 2024, ShiXin Wang. All wrongs reserved.

#include "toolpex/assert.h"
#include "koios/iouring_op_peripheral.h"

namespace koios::uring
{

op_peripheral_element::
op_peripheral_element(op_peripheral_element&& other) noexcept
    : m_buffer{ ::std::exchange(other.m_buffer, nullptr) }, 
      m_deleter{ other.m_deleter }
{
}

op_peripheral_element& 
op_peripheral_element::
operator=(op_peripheral_element&& other) noexcept
{
    this->delete_this();
    m_buffer = ::std::exchange(other.m_buffer, nullptr);
    m_deleter = other.m_deleter;
    return *this;
}

op_peripheral_element::
~op_peripheral_element() noexcept
{
    const ::std::size_t sz = this->delete_this();
    m_alloc.deallocate(::std::exchange(m_buffer, nullptr), sz);
}

::std::size_t
op_peripheral_element::
delete_this() noexcept
{
    toolpex_assert(m_deleter);
    if (m_buffer) return m_deleter(m_buffer);
    return 0;
}

} // namespace koios::uring
