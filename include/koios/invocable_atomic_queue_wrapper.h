#ifndef KOIOS_INVOCABLE_ATOMIC_QUEUE_WRAPPER_H
#define KOIOS_INVOCABLE_ATOMIC_QUEUE_WRAPPER_H

#include "atomic_queue/atomic_queue.h"
#include <cassert>
#include <optional>
#include <limits>

namespace koios
{

class invocable_atomic_queue_wrapper
{
public:
    using invocable_type = ::std::move_only_function<void()>;

public:
    invocable_atomic_queue_wrapper(size_t capacity)
        : m_q{ static_cast<unsigned int>(capacity) }
    {
        assert(capacity < ::std::numeric_limits<unsigned int>::max());
        assert(capacity > 0);
    }

    ::std::optional<invocable_type> 
    dequeue() noexcept
    {
        invocable_type result;
        if (m_q.try_pop(result))
            return result;
        return {};
    }

    void enqueue(invocable_type v)
    {
        m_q.push(::std::move(v));
    }

    bool empty() const noexcept
    {
        return m_q.was_empty();
    }

    size_t size() const noexcept
    {
        return m_q.was_size();
    }

private:
    atomic_queue::AtomicQueueB2<invocable_type> m_q;   
};

} // namespace koios

#endif
