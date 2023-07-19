#ifndef KOIOS_STD_QUEUE_WRAPPER_H
#define KOIOS_STD_QUEUE_WRAPPER_H

#include <queue>
#include <mutex>
#include "koios/macros.h"

KOIOS_NAMESPACE_BEG

struct std_queue_wrapper
{
public:
    using invocable_type = ::std::move_only_function<void()>;
    using queue_type = ::std::queue<invocable_type>;

    std_queue_wrapper() = default;

    void enqueue(invocable_type&& func)
    {
        ::std::unique_lock lk{ m_lock };
        m_q.emplace(::std::move(func));
    }

    ::std::optional<invocable_type> dequeue()
    {
        ::std::unique_lock lk{ m_lock };
        if (m_q.empty()) return {};
        auto result = ::std::move(m_q.front());
        m_q.pop();
        return { ::std::move(result) };
    }

    bool empty() const noexcept 
    { 
        ::std::unique_lock lk{ m_lock };
        return m_q.empty(); 
    }

    std_queue_wrapper(std_queue_wrapper&& other) noexcept
        : m_q{ ::std::move(other.m_q) }
    {
    }

private:
    queue_type m_q;   
    mutable ::std::mutex m_lock;
};

KOIOS_NAMESPACE_END

#endif
