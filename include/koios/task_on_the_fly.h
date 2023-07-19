#ifndef KOIOS_TASK_ON_THE_FLY_H
#define KOIOS_TASK_ON_THE_FLY_H

#include <coroutine>

#include "macros.h"

KOIOS_NAMESPACE_BEG

class task_on_the_fly
{
public:
    task_on_the_fly(::std::coroutine_handle<> h) 
        : m_h{ h }
    {
    }

    void operator()() const { m_h.resume(); }

    ~task_on_the_fly() noexcept
    {
        if (m_h.done()) m_h.destroy();
    }

private:
    ::std::coroutine_handle<> m_h;
};

KOIOS_NAMESPACE_END

#endif
