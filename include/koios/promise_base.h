#ifndef TASK_PROMISE_BASE_H
#define TASK_PROMISE_BASE_H

#include <coroutine>

#include "koios/macros.h"
#include "koios/local_thread_scheduler.h"
#include "koios/global_task_scheduler.h"
#include "koios/task_scheduler_wrapper.h"

KOIOS_NAMESPACE_BEG

class destroy_aw 
{
public:
    constexpr bool await_ready() const noexcept { return false; }
    void await_suspend(::std::coroutine_handle<> h) const noexcept { h.destroy(); }
    constexpr void await_resume() const noexcept { }
};

class promise_base
{
public:
    constexpr ::std::suspend_always initial_suspend() const noexcept
        { return {}; }
    constexpr destroy_aw final_suspend() const noexcept
        { return {}; }
    void unhandled_exception() const { throw; }
};

KOIOS_NAMESPACE_END

#endif
