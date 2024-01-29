#ifndef KOIOS_ASYNC_AWAITING_HUB_H
#define KOIOS_ASYNC_AWAITING_HUB_H

#include "koios/macros.h"
#include "koios/task_on_the_fly.h"
#include "koios/runtime.h"
#undef BLOCK_SIZE
#include "concurrentqueue/concurrentqueue.h"

KOIOS_NAMESPACE_BEG

class async_awaiting_hub
{
public:
    void add_awaiting(task_on_the_fly y) noexcept
    {
        m_awaitings.enqueue(::std::move(y));
    }

    void may_wake_next() noexcept
    {
        task_on_the_fly f{};
        if (!m_awaitings.try_dequeue(f))
            return;
        [[assume(bool(f))]];
        get_task_scheduler().enqueue(::std::move(f));       
    }
    
private:
    moodycamel::ConcurrentQueue<task_on_the_fly> m_awaitings;
};

KOIOS_NAMESPACE_END

#endif
