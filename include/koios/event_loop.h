/* Koios, A c++ async runtime library.
 * Copyright (C) 2023  Jeremy Pewterschmidt
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

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
class event_loop : public task_scheduler, private Loops...                
{
public:
    template<typename... Args>
    event_loop(Args&&... args)
        : task_scheduler(::std::forward<Args>(args)...)
    {
    }

    /*! \brief Call the function with same name of each event loop.
     *
     *  By the same order of `Loops` template arguments list.
     *
     *  \attention  We assume that this function will return immediately, 
     *              any blocking subcall are prohibited.
     */
    void do_occured_nonblk() noexcept
    {
        (this->Loops::do_occured_nonblk(), ...);
    }

    /*! \brief Add event to specific event loop. 
     *  \tparam SpecificLoop the loop you want to operate.
     *  \return something determined by the actual event loop.
     *
     *  This function typically forward all the arguments 
     *  to the same name member function of the determined loop.
     *  But only when the whole event loop are not in the cleanning mode (going to die).
     *  Or it throws an exception.
     */
    template<typename SpecificLoop>
    auto add_event(auto&&... data)
    {
        if (m_cleanning.load()) [[unlikely]]
        {
            throw koios::thread_pool_stopped_exception{ 
                "event_loop has gotten into destruction phase, "
                "refuse further add_event operation!"
            };
        }
        return SpecificLoop::add_event(::std::forward<decltype(data)>(data)...);
    }

    /*! \brief Get the reference of this determined loop. */
    template<typename Loop>
    auto& as_loop()
    {
        return static_cast<Loop&>(*this);
    }

    /*! \brief  Stop receiving any other event
     *  
     *  And call this function would stop all the sub eventloop working, 
     *  prohibit all the subsequent `add_event()` operation.
     */
    virtual void stop() noexcept override
    {
        m_cleanning = true;
        (Loops::stop(), ...);
        task_scheduler::stop();
    }

    void quick_stop() noexcept
    {
        (Loops::quick_stop(), ...);
        task_scheduler::quick_stop();
    }

    virtual ~event_loop() noexcept 
    {
        // Prevent some event_loop destructed 
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
