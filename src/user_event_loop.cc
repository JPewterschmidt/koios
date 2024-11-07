// This file is part of Koios
// https://github.com/JPewterschmidt/koios
//
// Copyleft 2023 - 2024, ShiXin Wang. All wrongs reserved.

#include "koios/user_event_loops.h"
#include "koios/user_event_loop_interface.h"
#include "toolpex/exceptions.h"

KOIOS_NAMESPACE_BEG

void user_event_loops::thread_specific_preparation(const per_consumer_attr& attr) noexcept
{
    ::std::unique_lock lk{ m_mutex };
    m_attrs.insert({ attr.thread_id, &attr });
    for (auto& loop : m_loops)
    {
        loop->thread_specific_preparation(attr);
    }
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
    m_cleanning = true;
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
    if (m_cleanning == true) [[unlikely]] return;
    for (auto& loop : m_loops)
    {
        loop->do_occured_nonblk();
    }
}

void user_event_loops::add_loop(user_event_loop_interface::sptr loop)
{
    ::std::unique_lock lk{ m_mutex };
    auto& inplace_loop = m_loops.emplace_back(::std::move(loop));
    for (auto [tid, attrp] : m_attrs)
    {
        inplace_loop->thread_specific_preparation(*attrp);
    }
}

bool user_event_loops::empty() const
{
    ::std::shared_lock lk{ m_mutex };
    for (const auto& loop : m_loops)
    {
        if (!loop->empty())
            return false;
    }
    return true;
}

KOIOS_NAMESPACE_END
