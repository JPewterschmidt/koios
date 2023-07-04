#ifndef KOIOS_TINY_TASK_H
#define KOIOS_TINY_TASK_H

#include "macros.h"
#include <coroutine>
#include <utility>

KOIOS_NAMESPACE_BEG

class tiny_task
{
public:
    tiny_task(::std::coroutine_handle<> h) noexcept
        : m_h{ ::std::move(h) }
    {
    }

    ~tiny_task() noexcept
    {
        if (m_h != nullptr && m_h.done()) 
            m_h.destroy();
    }

    tiny_task(tiny_task&& other)
        : m_h{ ::std::exchange(other.m_h, nullptr) }
    {
    }

    void operator()() const { if (m_h != nullptr) [[likely]] m_h(); }

private:
    ::std::coroutine_handle<> m_h;
};

KOIOS_NAMESPACE_END

#endif
