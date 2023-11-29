#ifndef KOIOS_EVENT_LOOP_H
#define KOIOS_EVENT_LOOP_H

#include <concepts>
#include <chrono>
#include <tuple>
#include <algorithm>
#include <atomic>

#include "koios/macros.h"
#include "koios/task_scheduler.h"
#include "toolpex/is_specialization_of.h"

KOIOS_NAMESPACE_BEG

template<::std::default_initializable... Loops>
class event_loop : public task_scheduler, public Loops...                
{
public:
    template<typename... Args>
    event_loop(Args&&... args)
        : task_scheduler(::std::forward<Args>(args)...)
    {
    }

    virtual void thread_specific_preparation() override
    {
        (Loops::thread_specific_preparation(), ...);
    }

    void do_occured_nonblk() noexcept
    {
        (Loops::do_occured_nonblk(), ...);
    }

    template<typename SpecificLoop>
    void add_event(auto&&... data)
    {
        //if (m_stop_requested.value()) [[unlikely]]
        //{
        //    throw koios::thread_pool_stopped_exception{ 
        //        "event_loop has gotten into destruction phase, "
        //        "refuse further add_event operation!"
        //    };
        //}
        SpecificLoop::add_event(::std::forward<decltype(data)>(data)...);
    }

    virtual void stop() noexcept override
    {
        (Loops::stop(), ...);
        task_scheduler::stop();
    }

    virtual ~event_loop() noexcept 
    {
        // TODO Prevent some event_loop destructed 
        // when control follow run out of the domain of main function
        // while runtime has been configured as manually_stop.
    }
    
private:
    virtual void before_each_task() noexcept override
    {
        do_occured_nonblk();
    }

    virtual ::std::chrono::nanoseconds
    max_sleep_duration() noexcept override
    {
        return ::std::min({
            (::std::chrono::duration_cast<::std::chrono::nanoseconds>(Loops::max_sleep_duration()), ...)
        });
    }
    
protected:
    event_loop(event_loop&&) noexcept = default;
    event_loop& operator=(event_loop&&) noexcept = default;

private:
    //::std::atomic_bool m_stop_requested{false};
};

KOIOS_NAMESPACE_END

#endif
