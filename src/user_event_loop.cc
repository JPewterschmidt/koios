/* koios, a c++ async runtime library.
 * copyright (c) 2024  jeremy pewterschmidt
 * 
 * this program is free software; you can redistribute it and/or
 * modify it under the terms of the gnu general public license
 * as published by the free software foundation; either version 2
 * of the license, or (at your option) any later version.
 * 
 * this program is distributed in the hope that it will be useful,
 * but without any warranty; without even the implied warranty of
 * merchantability or fitness for a particular purpose.  see the
 * gnu general public license for more details.
 * 
 * you should have received a copy of the gnu general public license
 * along with this program; if not, write to the free software
 * foundation, inc., 51 franklin street, fifth floor, boston, ma  02110-1301, usa.
 */

#include "koios/user_event_loops.h"
#include "koios/user_event_loop_interface.h"
#include "toolpex/exceptions.h"
#include <cassert>

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

KOIOS_NAMESPACE_END
