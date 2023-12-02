#ifndef KOIOS_DELAYED_SCHEDULER_H
#define KOIOS_DELAYED_SCHEDULER_H

#include "koios/timer.h"
#include "koios/runtime.h"

#include <chrono>

KOIOS_NAMESPACE_BEG

template<typename Rep, typename Period>
class delayed_scheduler
{
public:
    constexpr delayed_scheduler(::std::chrono::duration<Rep, Period> dura)
        : m_dura{ dura }
    {
    }

    void enqueue(task_concept auto t) noexcept { enqueue(::std::move(t)); }

    void enqueue(task_on_the_fly f) noexcept
    {
        get_task_scheduler().add_event<timer_event_loop>(m_dura, ::std::move(f));
    }

private:
    const ::std::chrono::duration<Rep, Period> m_dura;
};

KOIOS_NAMESPACE_END

#endif
