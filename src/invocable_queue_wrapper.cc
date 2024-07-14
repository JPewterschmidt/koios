// This file is part of Koios
// https://github.com/JPewterschmidt/koios
//
// Copyleft 2023 - 2024, ShiXin Wang. All wrongs reserved.

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

KOIOS_NAMESPACE_END
