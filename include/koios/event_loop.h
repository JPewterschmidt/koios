#ifndef KOIOS_EVENT_LOOP_H
#define KOIOS_EVENT_LOOP_H

#include <concepts>
#include <chrono>
#include <tuple>
#include <algorithm>

#include "koios/macros.h"
#include "koios/task_scheduler.h"

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

    void do_occured_nonblk() noexcept
    {
        (Loops::do_occured_nonblk(), ...);
    }

    template<typename SpecificLoop>
    void add_event(auto&& data)
    {
        SpecificLoop::add_event(::std::forward<decltype(data)>(data));
    }

    virtual ~event_loop() noexcept {}
    
private:
    void before_each_task() noexcept override
    {
        do_occured_nonblk();
    }

    ::std::chrono::nanoseconds
    max_sleep_duration() noexcept override
    {
        return ::std::min({
            (::std::chrono::duration_cast<::std::chrono::nanoseconds>(Loops::max_sleep_duration()), ...)
        });
    }
    
protected:
    event_loop(event_loop&&) noexcept = default;
    event_loop& operator=(event_loop&&) noexcept = default;
};

KOIOS_NAMESPACE_END

#endif
