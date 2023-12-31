#include <utility>

#include "koios/invocable_queue_wrapper.h"

KOIOS_NAMESPACE_BEG

invocable_queue_wrapper::
~invocable_queue_wrapper() noexcept
{
    if (m_dtor) m_dtor(m_storage.get());
}

invocable_queue_wrapper::
invocable_queue_wrapper(invocable_queue_wrapper&& other) noexcept
    : m_storage     { ::std::move(other.m_storage)              }, 
      m_dtor        { ::std::exchange(other.m_dtor, nullptr)    }, 
      m_empty_impl  { other.m_empty_impl                        },
      m_enqueue_impl{ other.m_enqueue_impl                      }, 
      m_dequeue_impl{ other.m_dequeue_impl                      }, 
      m_size_impl   { other.m_size_impl                         }, 
      m_thread_specific_preparation{ other.m_thread_specific_preparation }
{
}

bool
invocable_queue_wrapper::
empty() const
{
    return m_empty_impl(m_storage.get());
}

void 
invocable_queue_wrapper::
enqueue(invocable_type&& func) const
{ 
    m_enqueue_impl(m_storage.get(), nullptr, ::std::move(func)); 
}

void 
invocable_queue_wrapper::
enqueue(const per_consumer_attr& ca, invocable_type&& func) const
{ 
    m_enqueue_impl(m_storage.get(), &ca, ::std::move(func)); 
}

::std::optional<typename invocable_queue_wrapper::invocable_type> 
invocable_queue_wrapper::
dequeue(const per_consumer_attr& attr) const
{ 
    return m_dequeue_impl(m_storage.get(), attr); 
}

KOIOS_NAMESPACE_END
