#ifndef KOIOS_TIMER_H
#define KOIOS_TIMER_H

#include <chrono>
#include <memory>
#include <vector>
#include <utility>
#include <compare>
#include <mutex>

#include "koios/macros.h"
#include "koios/task_on_the_fly.h"
#include "koios/task_concepts.h"
#include "koios/exceptions.h"

KOIOS_NAMESPACE_BEG

struct timer_event
{
    ::std::chrono::time_point<::std::chrono::high_resolution_clock> timeout_tp;
    task_on_the_fly task;
    friend ::std::strong_ordering 
    operator<=>(const timer_event& lhs, const timer_event& rhs) noexcept;
};

class timer_event_loop_impl
{
public:
    void do_occured_nonblk() noexcept;

    void add_event(auto dura, task_on_the_fly f) noexcept
    {
        const auto now = ::std::chrono::high_resolution_clock::now();
        add_event_impl({ now + dura, ::std::move(f) });
    }

    void add_event(auto dura, task_concept auto t) noexcept
    {
        auto to_tof = [](auto t){ 
            task_on_the_fly result = t;
            return result;
        };
        add_event(dura, to_tof(::std::move(t)));
    }

    auto max_sleep_duration() noexcept
    {
        const auto now = ::std::chrono::high_resolution_clock::now();
        ::std::unique_lock lk{ m_lk };
        if (m_timer_heap.empty()) return ::std::chrono::nanoseconds::max();
        return m_timer_heap.front().timeout_tp - now;
    }
    
    void stop() noexcept { m_timer_heap.clear(); }

private:
    void add_event_impl(timer_event te) noexcept;

private:
    ::std::vector<timer_event> m_timer_heap;
    mutable ::std::mutex m_lk;
};

class timer_event_loop
{
public:
    timer_event_loop()
        : m_impl_ptr { ::std::make_shared<timer_event_loop_impl>() }
    {
    }

    void do_occured_nonblk() noexcept 
        { m_impl_ptr->do_occured_nonblk(); }

    template<typename... Args>
    void add_event(Args&&... args) noexcept 
        { m_impl_ptr->add_event(::std::forward<Args>(args)...); } 

    auto max_sleep_duration() noexcept
        { return m_impl_ptr->max_sleep_duration(); }

    void stop() noexcept { m_impl_ptr->stop(); }

    void thread_specific_preparation()
    {
        thread_local auto prevent_heap_used_after_free = m_impl_ptr;
        (void)prevent_heap_used_after_free;
    }

private:    
    ::std::shared_ptr<timer_event_loop_impl> m_impl_ptr;
};

KOIOS_NAMESPACE_END

#endif
