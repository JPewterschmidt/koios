// This file is part of Koios
// https://github.com/JPewterschmidt/koios
//
// Copyleft 2023 - 2024, ShiXin Wang. All wrongs reserved.

#include "koios/timer.h"
#include "koios/runtime.h"
#include "koios/functional.h"

#include <algorithm>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <thread>

KOIOS_NAMESPACE_BEG

bool timer_event_loop_impl::
do_occured_nonblk() noexcept
{
    const auto now = ::std::chrono::high_resolution_clock::now();

    if (m_stop_tk.stop_requested()) 
    {
        return {};
    }

    ::std::shared_lock lk{ m_lk };
    if (m_timer_heap.empty() 
        || now < m_timer_heap.front().timeout_tp)
    {
        return {};
    }
    lk.unlock();

    ::std::unique_lock ulk{ m_lk };
    if (m_timer_heap.empty())
    {
        return {};
    }

    auto heap_end = m_timer_heap.end();
    while (now >= m_timer_heap.front().timeout_tp)
    {
        ::std::pop_heap(m_timer_heap.begin(), heap_end, 
                        ::std::greater<timer_event>{});
        if (--heap_end == m_timer_heap.begin()) break;
    }
    auto& schr = get_task_scheduler();
    for (auto i{ heap_end }; i < m_timer_heap.end(); ++i)
    {
        schr.enqueue(::std::move(i->task));
    }
    m_timer_heap.erase(heap_end, m_timer_heap.end());

    return true;
}

void timer_event_loop_impl::
wake_up_all() noexcept
{
    ::std::unique_lock ulk{ m_lk };
    auto& schr = get_task_scheduler();
    for (auto& timer_ev : m_timer_heap)
    {
        schr.enqueue(::std::move(timer_ev.task));
    }
    m_timer_heap.clear();
}

void timer_event_loop_impl::
add_event_impl(timer_event te) noexcept
{
    ::std::unique_lock lk{ m_lk };
    m_timer_heap.emplace_back(::std::move(te));
    ::std::push_heap(
        m_timer_heap.begin(), 
        m_timer_heap.end(), 
        ::std::greater<timer_event>{}
    );
}

void timer_event_loop::
quick_stop() noexcept
{
    ::std::unique_lock lk{ m_ptrs_lock };
    for (auto& [k, ptrs] : m_impl_ptrs)
    {
        ptrs->quick_stop();
    }
    this->stop();
}

bool timer_event_loop::
done() const
{
    ::std::shared_lock lk{ m_ptrs_lock };
    for (auto& [k, ptr] : m_impl_ptrs)
    {
        if (!ptr->done()) 
            return false;
    }
    return true;
}

bool timer_event_loop::empty() const
{
    return done();
}

::std::strong_ordering 
operator<=>(const timer_event& lhs, const timer_event& rhs) noexcept
{
    return lhs.timeout_tp <=> rhs.timeout_tp;
}

KOIOS_NAMESPACE_END
