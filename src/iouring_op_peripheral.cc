#include "koios/iouring_op_peripheral.h"
#include <cassert>

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
    delete_this();
    m_buffer = ::std::move(other.m_buffer);
    m_deleter = other.m_deleter;
    other.m_buffer = nullptr;
    return *this;
}

op_peripheral_element::
~op_peripheral_element() noexcept
{
    delete_this();
}

void 
op_peripheral_element::
delete_this() noexcept
{
    assert(m_deleter);
    if (m_buffer) m_deleter(m_buffer.get());
}

} // namespace koios::uring
