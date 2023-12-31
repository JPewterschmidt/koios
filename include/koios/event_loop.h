#ifndef KOIOS_EVENT_LOOP_H
#define KOIOS_EVENT_LOOP_H

#include <concepts>
#include <chrono>
#include <tuple>
#include <algorithm>
#include <atomic>
#include <ranges>

#include "koios/macros.h"
#include "koios/task_scheduler.h"
#include "toolpex/is_specialization_of.h"
#include "koios/event_loop_concepts.h"
#include "koios/per_consumer_attr.h" 

KOIOS_NAMESPACE_BEG

/*! \brief Eventloop class type
 *
 *  This type inherited from `task_scheduler`, `thread_pool`
 *  and bunch of sub-event loop, the core loop in `thread_pool::consumer`
 *  would call the virtual function `before_each_task()` periodically, 
 *  and this function would drive the whole event loop 
 *  (all the sub event loop) work.
 *
 *  There're two types of event, one needs to be driven (by this class), 
 *  the other would do the job by itself.
 *  Both types hold ownership of `task_on_the_fly`.
 */
template<event_loop_concept... Loops>
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

    /*! \brief Add event to specific event loop. 
     *  \tparam SpecificLoop the loop you want to operate.
     */
    template<typename SpecificLoop>
    void add_event(auto&&... data)
    {
        if (m_cleanning.load()) [[unlikely]]
        {
            throw koios::thread_pool_stopped_exception{ 
                "event_loop has gotten into destruction phase, "
                "refuse further add_event operation!"
            };
        }
        SpecificLoop::add_event(::std::forward<decltype(data)>(data)...);
    }

    /*! \brief  Stop receiving any other event
     *  
     *  And call this function would stop all the sub eventloop working.
     */
    virtual void stop() noexcept override
    {
        m_cleanning = true;
        (Loops::stop(), ...);
        task_scheduler::stop();
    }

    virtual ~event_loop() noexcept 
    {
        // TODO Prevent some event_loop destructed 
        // when control follow run out of the domain of main function
        // while runtime has been configured as manually_stop.
        
        m_cleanning = true;
        if (need_stop_now()) (Loops::quick_stop(), ...);
        else                 (Loops::stop(), ...);
        (Loops::until_done(), ...);
    }
    
protected:
    virtual void before_each_task() noexcept override
    {
        do_occured_nonblk();
    }

    virtual ::std::chrono::nanoseconds
    max_sleep_duration(const per_consumer_attr& cattr) noexcept override
    {
        static ::std::vector<::std::chrono::nanoseconds> duras(sizeof...(Loops));
        duras.clear();
        (duras.push_back(
            ::std::chrono::duration_cast<::std::chrono::nanoseconds>(
                Loops::max_sleep_duration(cattr))), 
        ...);
        auto it = ::std::ranges::min_element(duras);
        if (it == duras.end()) [[unlikely]] return ::std::chrono::nanoseconds::max();
        return *it;
    }

    event_loop(event_loop&&) noexcept = default;
    event_loop& operator=(event_loop&&) noexcept = default;

    virtual void thread_specific_preparation(const per_consumer_attr& attr) override
    {
        task_scheduler::thread_specific_preparation(attr);
        (Loops::thread_specific_preparation(attr), ...);
    }

private:
    ::std::atomic_bool m_cleanning{false};
};

KOIOS_NAMESPACE_END

#endif
