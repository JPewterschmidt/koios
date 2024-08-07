// This file is part of Koios
// https://github.com/JPewterschmidt/koios
//
// Copyleft 2023 - 2024, ShiXin Wang. All wrongs reserved.

#include "koios/moodycamel_queue_wrapper.h"

KOIOS_NAMESPACE_BEG

void 
moodycamel_queue_wrapper::
enqueue(invocable_type&& func)
{
    m_q.enqueue(::std::move(func));
}

::std::optional<moodycamel_queue_wrapper::invocable_type> 
moodycamel_queue_wrapper::
dequeue()
{
    invocable_type func;
    if (!m_q.try_dequeue(func))
        return {};
    return func;
}

void 
moodycamel_queue_wrapper::
enqueue(const per_consumer_attr& ca, invocable_type&& func)
{
    m_q.enqueue(::std::move(func));
}

::std::optional<moodycamel_queue_wrapper::invocable_type> 
moodycamel_queue_wrapper::
dequeue(const per_consumer_attr& ca)
{
    invocable_type func;
    if (!m_q.try_dequeue(func))
        return {};
    return func;
}

bool
moodycamel_queue_wrapper::
empty() const
{
    return m_q.size_approx() == 0;
}

KOIOS_NAMESPACE_END
