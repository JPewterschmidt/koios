#ifndef KOIOS_THIS_TASK_H
#define KOIOS_THIS_TASK_H

#include "macros.h"
#include "runtime.h"
#include <chrono>

KOIOS_NAMESPACE_BEG

namespace this_task
{
    template<typename Duration>
    class sleep_await
    {
    public:
        sleep_await(Duration dura)
            : m_dura{ dura }
        {
        }

        bool await_ready() const noexcept
        {
            return m_dura == Duration{};
        }

        void await_suspend(task_on_the_fly h) const 
        {
            get_task_scheduler().add_event<timer_event_loop>(m_dura, ::std::move(h));
        }

        constexpr void await_resume() const noexcept { }
        
    private:
        Duration m_dura;
    };

    auto sleep_for(auto dura)
    {
        return sleep_await{ dura };
    }
}

KOIOS_NAMESPACE_END

#endif
