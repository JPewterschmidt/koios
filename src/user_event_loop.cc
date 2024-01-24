#include "koios/user_event_loops.h"
#include <cassert>

KOIOS_NAMESPACE_BEG

void user_event_loops::thread_specific_preparation(const per_consumer_attr& attr) noexcept
{
    ::std::unique_lock lk{ m_mutex };
    for (auto& loop : m_loops)
    {
        loop->thread_specific_preparation(attr);
    }
    m_after_ts_prep = true;
}

void user_event_loops::stop() noexcept
{
    ::std::shared_lock lk{ m_mutex };
    for (auto& loop : m_loops)
    {
        loop->stop();
    }
}

void user_event_loops::quick_stop() noexcept
{
    ::std::shared_lock lk{ m_mutex };
    for (auto& loop : m_loops)
    {
        loop->quick_stop();
    }
}

void user_event_loops::until_done()
{
    ::std::shared_lock lk{ m_mutex };
    for (auto& loop : m_loops)
    {
        loop->until_done();
    }
}

::std::chrono::milliseconds 
user_event_loops::max_sleep_duration(const per_consumer_attr& attr) noexcept
{
    ::std::chrono::milliseconds result{10000};
    ::std::shared_lock lk{ m_mutex };
    for (auto& loop : m_loops)
    {
        result = ::std::min(loop->max_sleep_duration(attr), result);
    }
    return result;
}

void user_event_loops::do_occured_nonblk() noexcept
{
    ::std::shared_lock lk{ m_mutex };
    for (auto& loop : m_loops)
    {
        loop->do_occured_nonblk();
    }
}

void user_event_loops::add_loop(user_event_loop::uptr loop)
{
    ::std::unique_lock lk{ m_mutex };
    assert(!m_after_ts_prep && !!loop);
    m_loops.push_back(::std::move(loop));
}

KOIOS_NAMESPACE_END
