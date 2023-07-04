#ifndef KOIOS_TASK_SCHEDULER_WRAPPER_H
#define KOIOS_TASK_SCHEDULER_WRAPPER_H

#include <coroutine>
#include "macros.h"

KOIOS_NAMESPACE_BEG

class task_scheduler_wrapper
{
public:
    template<typename Schr>
    task_scheduler_wrapper(Schr& scheduler) noexcept
        : m_enqueue_impl { 
            [](void* schr, ::std::coroutine_handle<> h) mutable { 
                static_cast<Schr*>(schr)->enqueue(h); 
            } 
          }, 
          m_schr{ &scheduler }
    {
    }

    void enqueue(::std::coroutine_handle<> h) 
    {
        m_enqueue_impl(m_schr, h);
    }

private:
    void (*m_enqueue_impl)(void*, ::std::coroutine_handle<>);
    void* const m_schr{};
};

KOIOS_NAMESPACE_END

#endif
