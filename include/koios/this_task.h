// This file is part of Koios
// https://github.com/JPewterschmidt/koios
//
// Copyleft 2023 - 2024, ShiXin Wang. All wrongs reserved.

#ifndef KOIOS_THIS_TASK_H
#define KOIOS_THIS_TASK_H

#include "koios/macros.h"
#include "koios/runtime.h"
#include "koios/get_id_aw.h"
#include "koios/task.h"
#include <chrono>

#include "toolpex/concepts_and_traits.h"

KOIOS_NAMESPACE_BEG

namespace this_task
{
    /*! \brief  Return a awaitable object for getting the task id. */
    inline get_id_aw get_id() noexcept { return {}; }

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

    /*! \brief  Awatiable: Yield the current task, after user determined duration, 
     *          the `timer_event_loop` will wake this task up.
     *
     *          Similary to ::std::this_task::sleep_for.
     */
    auto sleep_for(toolpex::is_specialization_of<::std::chrono::duration> auto dura)
    {
        return sleep_await{ dura };
    }

    /*! \brief  Awatiable: Yield the current task, after user determined time point,
     *          the `timer_event_loop` will wake this task up.
     *
     *          Similary to ::std::this_task::sleep_until.
     */
    template<typename Clock, typename Duration = typename Clock::duration>
    auto sleep_until(::std::chrono::time_point<Clock, Duration> tp)
    {
        auto dura = tp - Clock::now();
        dura = dura > Duration{} ? dura : Duration{};
        return sleep_await{ dura };
    }

    /*! \brief  Awatiable: Yield the current task. 
     *          
     *          Guarantee the yielded task will go through the `task_scheduler`.
     *          Similary to `::std::this_task::yield`.
     */
    inline auto yield()
    {
        class yield_aw
        {
        public:
            constexpr bool await_ready() const noexcept { return false; }
            void await_suspend(task_on_the_fly f) const noexcept { get_task_scheduler().enqueue(::std::move(f)); }
            constexpr void await_resume() const noexcept {}
        };
        return yield_aw{};
    }

    task<> turn_into_scheduler();
}

KOIOS_NAMESPACE_END

#endif
