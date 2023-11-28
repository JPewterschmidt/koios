#include "koios/timer.h"
#include "koios/runtime.h"

#include <algorithm>
#include <functional>

KOIOS_NAMESPACE_BEG

void timer_event_loop_impl::
do_occured_nonblk() noexcept
{
    const auto now = ::std::chrono::high_resolution_clock::now();
    if (m_timer_heap.empty()) return;

    auto& nearest = m_timer_heap.front();
    if (now < nearest.timeout_tp)
        return;

    auto it = prev(m_timer_heap.end());
    for (;now >= it->timeout_tp; --it)
    {
        ::std::pop_heap(m_timer_heap.begin(), it,
                ::std::greater<timer_event>{});
    }
    auto& schr = get_task_scheduler();
    for (auto cur = it; cur < m_timer_heap.end(); ++cur)
    {
        schr.enqueue(::std::move(cur->task));
    }
    m_timer_heap.erase(it, m_timer_heap.end());
}

void timer_event_loop_impl::
add_event(timer_event te) noexcept
{
    m_timer_heap.emplace_back(::std::move(te));
    ::std::push_heap(m_timer_heap.begin(), m_timer_heap.end(), 
            ::std::greater<timer_event>{});
}

::std::strong_ordering 
operator<=>(const timer_event& lhs, const timer_event& rhs) noexcept
{
    return lhs.timeout_tp <=> rhs.timeout_tp;
}

KOIOS_NAMESPACE_END
