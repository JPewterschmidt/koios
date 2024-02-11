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

#include "koios/iouring.h"
#include "koios/runtime.h"
#include <cassert>
#include <thread>
#include <chrono>
#include "toolpex/tic_toc.h"
#include "koios/task.h"
#include "koios/functional.h"
#include "koios/iouring_awaitables.h"

KOIOS_NAMESPACE_BEG

using namespace ::std::chrono_literals;

namespace iel_detials
{
    void ioret_task::set_ret_and_wakeup(int32_t ret, uint32_t flags)
    {
        m_task_container
            ->release()
            .and_then([=, this](auto&& t) mutable -> ::std::optional<task_on_the_fly> { 
                this->m_ret->ret = ret;
                this->m_ret->flags = flags;
                get_task_scheduler().enqueue(::std::move(t)); 
                return ::std::nullopt;
            });
    }

    void iouring_event_loop_perthr::
    dealwith_cqe(const ::io_uring_cqe* cqep)
    {
        assert(cqep);

        const uint64_t key = cqep->user_data;
        auto it = m_suspended.find(key);
        assert(it != m_suspended.end()); // XXX TODO Likely to failed when there's a timer
        auto cb = ::std::move(it->second);
        m_suspended.erase(it);

        cb.set_ret_and_wakeup(cqep->res, cqep->flags);
    }
    
    void iouring_event_loop_perthr::
    do_occured_nonblk() noexcept
    {
        ::io_uring_cqe* cqep{};
        auto lk = get_lk();
        m_shot_record <<= 1u;

        const size_t left = ::io_uring_cq_ready(&m_ring);
        if (left == 0) m_shot_record |= 1u;
        for (size_t i{}; i < left; ++i)
        {
            int e = ::io_uring_peek_cqe(&m_ring, &cqep);
            if (e == - EAGAIN) [[unlikely]] // no any CQE available
            {
                break;
            }
            dealwith_cqe(cqep);
            ::io_uring_cqe_seen(&m_ring, cqep); // mark this cqe has been processed
        }
    }

    ::std::shared_ptr<task_release_once> 
    iouring_event_loop_perthr::
    add_event(
        task_on_the_fly h, 
        ::std::shared_ptr<uring::ioret> retslot, 
        ::io_uring_sqe sqe)
    {
        const auto addr = reinterpret_cast<uint64_t>(h.address());
        ::io_uring_sqe_set_data64(&sqe, addr);
        auto lk = get_lk();
        ::io_uring_sqe* sqep = ::io_uring_get_sqe(&m_ring);

        ioret_task t{ ::std::move(retslot), ::std::move(h) };
        auto result = t.get_task_shrptr();
        
        assert(!m_suspended.contains(addr));
        m_suspended.insert({ addr, ::std::move(t) });
        *sqep = sqe;
        ::io_uring_submit(&m_ring);

        return result;
    }

    ::std::chrono::milliseconds 
    iouring_event_loop_perthr::
    max_sleep_duration() const
    {
        constexpr uint8_t mask = 2u;
        auto lk = get_lk();
        return static_cast<int>(mask & m_shot_record) * 50ms;
    }

    void iouring_event_loop_perthr::
    set_timeout(int fd, void* user_data, 
                ::std::chrono::system_clock::time_point timeout) noexcept
    {
        get_task_scheduler().add_event<timer_event_loop>(
            timeout, [] (int fd, void* key) -> emitter_task<> {
                co_await uring::cancel_any(fd, key);
            }(fd, user_data)
        );
    }
}

void iouring_event_loop::
thread_specific_preparation(const per_consumer_attr& attr)
{
    auto unilk = get_unilk();
    m_impls.insert({
        attr.thread_id, 
        ::std::make_unique<
            iel_detials::iouring_event_loop_perthr
        >()
    });
}

auto iouring_event_loop::
shrlk_and_curthr_ptr()
{
    const auto id = ::std::this_thread::get_id();
    auto lk = get_shrlk();
    auto* ptr = m_impls[id].get();
    if (!m_impls.contains(id))
    {
        ptr = m_impls.begin()->second.get();
    }
    return ::std::make_pair(::std::move(lk), ptr);
}

void iouring_event_loop::
do_occured_nonblk()
{
    auto [lk, ptr] = shrlk_and_curthr_ptr();
    if (m_cleaning == true) [[unlikely]] return;
    ptr->do_occured_nonblk();
}

::std::shared_ptr<task_release_once> 
iouring_event_loop::
add_event(
    task_on_the_fly h, 
    ::std::shared_ptr<uring::ioret> retslot, 
    ::io_uring_sqe sqe)
{
    const auto tid = ::std::this_thread::get_id();
    auto lk = get_shrlk();
    if (m_cleaning == true) [[unlikely]] return {};
    auto it = m_impls.find(tid);
    assert(it != m_impls.end());
    return it->second->add_event(
        ::std::move(h), ::std::move(retslot), ::std::move(sqe)
    );
}

void iouring_event_loop::
stop(::std::unique_lock<::std::shared_mutex> lk)
{
    m_cleaning = true;
}

void iouring_event_loop::
quick_stop()
{
    auto lk = get_unilk();
    decltype(m_impls) going_to_die;
    m_impls.swap(going_to_die);
    stop(::std::move(lk));
}

::std::chrono::milliseconds 
iouring_event_loop::
max_sleep_duration(const per_consumer_attr& attr) const 
{
    auto lk = get_shrlk();
    if (m_cleaning == true) [[unlikely]] return 10000ms;
    if (auto it = m_impls.find(attr.thread_id); it != m_impls.end())
        return ::std::min(200ms, it->second->max_sleep_duration());
    return {};
}

bool 
iel_detials::iouring_event_loop_perthr::
DEBUG_has_key(void* key)
{
    uint64_t val = (uint64_t)key;
    return m_suspended.contains(val);
}

iel_detials::iouring_event_loop_perthr* 
iouring_event_loop::
DEBUG_one_who_has_key(void* key)
{
    for (auto& [k, v] : m_impls)
    {
        if (v->DEBUG_has_key(key))
            return v.get();
    }
    return nullptr;
}

KOIOS_NAMESPACE_END
