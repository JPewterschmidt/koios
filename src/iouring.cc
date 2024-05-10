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
    void iouring_event_loop_perthr::
    mis_shot_this_time() noexcept 
    { 
        m_num_mis_shot <<= 1u;
    }

    void iouring_event_loop_perthr::
    shot_this_time() noexcept 
    { 
        m_num_mis_shot = 1u; 
    }

    void iouring_event_loop_perthr::
    dealwith_cqe(const ::io_uring_cqe* cqep)
    {
        assert(cqep);
        auto& schr = get_task_scheduler();

        const uintptr_t key = cqep->user_data;
        auto it = m_opreps.find(key);
        assert(it != m_opreps.end());
        auto& [batch_rep, task] = it->second;
        batch_rep->add_ret(cqep->res, cqep->flags);
        if (batch_rep->has_enough_ret())
        {
            auto t = ::std::move(task);
            m_opreps.erase(it);
            schr.enqueue(::std::move(t));
        }
    }
    
    void iouring_event_loop_perthr::
    do_occured_nonblk() noexcept
    {
        auto lk = get_lk();

        const size_t left = ::io_uring_cq_ready(&m_ring);
        if (left == 0) 
        {
            mis_shot_this_time();
            return;
        }
        shot_this_time();

        ::io_uring_cqe* cqep{};
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

    void 
    iouring_event_loop_perthr::
    add_event(task_on_the_fly h, uring::op_batch_rep& ops)
    {
        const uintptr_t addrkey = reinterpret_cast<uint64_t>(h.address());
        auto lk = get_lk();
        assert(!m_opreps.contains(addrkey));
        m_opreps.insert({addrkey, ::std::make_pair(&ops, ::std::move(h))});
        shot_this_time();
        for (const auto& sqe : ops)
            *::io_uring_get_sqe(&m_ring) = sqe;
        ::io_uring_submit(&m_ring);
    }

    ::std::chrono::milliseconds 
    iouring_event_loop_perthr::
    max_sleep_duration() const
    {
        auto lk = get_lk();
        return m_opreps.empty() * mis_shot_indicator() * 25ms;
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
    auto it = m_impls.find(id);
    return ::std::make_pair(::std::move(lk), (it != m_impls.end() ? it->second.get() : nullptr));
}

void iouring_event_loop::
do_occured_nonblk()
{
    auto [lk, ptr] = shrlk_and_curthr_ptr();
    if (!ptr && m_cleaning) [[unlikely]] return;
    else if (!ptr && !m_cleaning)
    {
        ::std::cout << 
            "you should call async uring operation "
            "in a eager_task or any subsequent normal task.";
        ::exit(1);
    }
    ptr->do_occured_nonblk();
}

void 
iouring_event_loop::
add_event(task_on_the_fly h, uring::op_batch_rep& ops)
{
    auto [lk, impl] = shrlk_and_curthr_ptr();
    if (!impl && m_cleaning) [[unlikely]] return;
    else if (!impl) 
    {
        ::std::cout << 
            "you should call async uring operation "
            "in a eager_task or any subsequent normal task.";
        ::exit(1);
    }
    return impl->add_event(::std::move(h), ops);
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

KOIOS_NAMESPACE_END
