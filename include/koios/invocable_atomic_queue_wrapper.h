#ifndef KOIOS_INVOCABLE_ATOMIC_QUEUE_WRAPPER_H
#define KOIOS_INVOCABLE_ATOMIC_QUEUE_WRAPPER_H

#include "atomic_queue/atomic_queue.h"
#include <cassert>
#include <optional>

namespace koios
{

class invocable_atomic_queue_wrapper
{
public:
    using invocable_type = ::std::move_only_function<void()>;

public:
    invocable_atomic_queue_wrapper(size_t capacity)
        : m_q{ capacity }
    {
        assert(capacity);
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
        // It won't actually move `v` if there's not enough space.
        while (!m_q.try_push(::std::move(v)))
            ;
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
    atomic_queue::AtomicQueueB<invocable_type> m_q;   
};

} // namespace koios

#endif
