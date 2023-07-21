#ifndef KOIOS_TASK_SCHEDULER_WRAPPER_H
#define KOIOS_TASK_SCHEDULER_WRAPPER_H

#include <utility>
#include <type_traits>

#include "koios/macros.h"
#include "koios/task_on_the_fly.h"

KOIOS_NAMESPACE_BEG

class task_scheduler_wrapper
{
public:
    using is_wrapper_t = ::std::true_type;

public:
    template<typename Schr>
    task_scheduler_wrapper(Schr& scheduler) noexcept
        : m_enqueue_impl { 
            [](void* schr, task_on_the_fly h) mutable { 
                static_cast<Schr*>(schr)->enqueue(::std::move(h)); 
            } 
          }, 
          m_schr{ &scheduler }
    {
    }

    void enqueue(task_on_the_fly h) 
    {
        m_enqueue_impl(m_schr, ::std::move(h));
    }

private:
    void (*m_enqueue_impl)(void*, task_on_the_fly);
    void* const m_schr{};
};

class task_scheduler_owned_wrapper
{
public:
    using is_wrapper_t = ::std::true_type;

public:
    template<typename Schr>
    task_scheduler_owned_wrapper(Schr scheduler) noexcept
        : m_enqueue_impl { 
            [](void* schr, task_on_the_fly h) mutable { 
                static_cast<Schr*>(schr)->enqueue(::std::move(h)); 
            } 
          }, 
          m_storage{ new unsigned char[sizeof(Schr)] }, 
          m_dtor{ [](void* schr){ static_cast<Schr*>(schr)->~Schr(); } }
    {
        new(m_storage.get()) Schr(::std::move(scheduler));
    }

    ~task_scheduler_owned_wrapper() noexcept
    {
        m_dtor(m_storage.get());
    }

    task_scheduler_owned_wrapper(task_scheduler_owned_wrapper&&) noexcept = default;
    task_scheduler_owned_wrapper(const task_scheduler_owned_wrapper&) = delete;

    void enqueue(task_on_the_fly h) 
    {
        m_enqueue_impl(m_storage.get(), ::std::move(h));
    }

private:
    void (*m_enqueue_impl)(void*, task_on_the_fly);
    ::std::unique_ptr<unsigned char[]> m_storage{};
    void (*m_dtor)(void*);
};


KOIOS_NAMESPACE_END

#endif
