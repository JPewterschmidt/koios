#ifndef TASK_PROMISE_BASE_H
#define TASK_PROMISE_BASE_H

#include "koios/macros.h"
#include <coroutine>

KOIOS_NAMESPACE_BEG

class promise_base
{
public:
    constexpr ::std::suspend_always initial_suspend() const noexcept
        { return {}; }
    constexpr ::std::suspend_always final_suspend() const noexcept
        { return {}; }
    void unhandled_exception() const { throw; }
};

KOIOS_NAMESPACE_END

#endif
