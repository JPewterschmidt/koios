#ifndef KOIOS_TIMER_H
#define KOIOS_TIMER_H

#include <chrono>
#include <memory>
#include <vector>
#include <utility>
#include <compare>

#include "koios/macros.h"
#include "koios/task_on_the_fly.h"

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
    void add_event(timer_event te) noexcept;

private:
    ::std::vector<timer_event> m_timer_heap;
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

    void add_event(timer_event event) noexcept 
        { m_impl_ptr->add_event(::std::move(event)); } 

private:    
    ::std::shared_ptr<timer_event_loop_impl> m_impl_ptr;
};

KOIOS_NAMESPACE_END

#endif
