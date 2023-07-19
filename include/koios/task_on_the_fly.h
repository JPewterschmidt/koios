#ifndef KOIOS_TASK_ON_THE_FLY_H
#define KOIOS_TASK_ON_THE_FLY_H

#include <coroutine>
#include <utility>

#include "macros.h"

KOIOS_NAMESPACE_BEG

class task_on_the_fly
{
public:
    task_on_the_fly(::std::coroutine_handle<> h) 
        : m_h{ h }
    {
    }

    task_on_the_fly(task_on_the_fly&& other) noexcept
        : m_h{ other.m_h }, m_need_destroy{ ::std::exchange(other.m_need_destroy, false) }
    {
    }

    void operator()()
    { 
        m_need_destroy = false;
        m_h.resume(); 
    }

    ~task_on_the_fly() noexcept
    {
        if (m_need_destroy) [[unlikely]] m_h.destroy();
    }

private:
    ::std::coroutine_handle<> m_h;
    bool m_need_destroy{ true };
};

KOIOS_NAMESPACE_END

#endif
