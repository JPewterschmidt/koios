// This file is part of Koios
// https://github.com/JPewterschmidt/koios
//
// Copyleft 2023 - 2024, ShiXin Wang. All wrongs reserved.

#include "toolpex/assert.h"

#include "koios/iouring.h"
#include "koios/runtime.h"
#include <thread>
#include <chrono>
#include <print>
#include <cstdio>
#include "toolpex/tic_toc.h"
#include "koios/task.h"
#include "koios/functional.h"
#include "koios/iouring_awaitables.h"

#include "spdlog/spdlog.h"

KOIOS_NAMESPACE_BEG

using namespace ::std::chrono_literals;

thread_local ::std::unique_ptr<iel_detials::iouring_event_loop_perthr> iouring_event_loop::m_impl{};

namespace iel_detials
{
    void iouring_event_loop_perthr::
    stop()
    {
        auto lk = get_lk();
        m_stoped = true;
    }

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
        toolpex_assert(cqep);
        auto& schr = get_task_scheduler();

        const uintptr_t key = cqep->user_data;
        auto it = m_opreps.find(key);
        toolpex_assert(it != m_opreps.end());
        auto& [batch_rep, task] = it->second;
        batch_rep->add_ret(cqep->res, cqep->flags);
        if (batch_rep->has_enough_ret())
        {
            auto t = ::std::move(task);
            m_opreps.erase(it);
            schr.enqueue(::std::move(t));
        }
    }
    
    bool iouring_event_loop_perthr::
    do_occured_nonblk() noexcept
    {
        auto lk = this->get_lk();
        if (m_stoped)
            return false;

        const size_t left = ::io_uring_cq_ready(&m_ring);
        if (left == 0) 
        {
            this->mis_shot_this_time();
            return false;
        }
        this->shot_this_time();

        ::io_uring_cqe* cqep{};
        for (size_t i{}; i < left; ++i)
        {
            int e = ::io_uring_peek_cqe(&m_ring, &cqep);
            if (e == - EAGAIN) [[unlikely]] // no any CQE available
            {
                break;
            }

            this->dealwith_cqe(cqep);
            ::io_uring_cqe_seen(&m_ring, cqep); // mark this cqe has been processed
        }
        return true;
    }

    bool iouring_event_loop_perthr::
    empty() const
    {
        auto lk = this->get_lk();
        return m_opreps.empty();
    }

    void 
    iouring_event_loop_perthr::
    add_event(task_on_the_fly h, uring::op_batch_rep& ops)
    {
        const uintptr_t addrkey = reinterpret_cast<uint64_t>(h.address());
        auto lk = this->get_lk();
        if (m_stoped) return;
        toolpex_assert(!m_opreps.contains(addrkey));
        m_opreps.insert({addrkey, ::std::make_pair(&ops, ::std::move(h))});
        this->shot_this_time();
        for (const auto& sqe : ops)
            *::io_uring_get_sqe(&m_ring) = sqe;
        ::io_uring_submit(&m_ring);
    }

    ::std::chrono::milliseconds 
    iouring_event_loop_perthr::
    max_sleep_duration() const
    {
        auto lk = this->get_lk();
        return m_opreps.empty() * this->mis_shot_indicator() * 25ms;
    }

    void iouring_event_loop_perthr::
    print_status() const
    {
        auto lk = this->get_lk();
        spdlog::info("iouring per-thread status: has {} event(s) pending{}.", m_opreps.size(), (m_stoped ? ", and stopped." : ""));
    }
}

void iouring_event_loop::
thread_specific_preparation(const per_consumer_attr& attr)
{
    m_impl = ::std::make_unique<iel_detials::iouring_event_loop_perthr>();
}

bool iouring_event_loop::
do_occured_nonblk()
{
    toolpex_assert(m_impl);
    return m_impl->do_occured_nonblk();
}

bool iouring_event_loop::
empty() const
{
    if (!m_impl) [[unlikely]]
        return true;
    return m_impl->empty();
}

void 
iouring_event_loop::
add_event(task_on_the_fly h, uring::op_batch_rep& ops)
{
    toolpex_assert(m_impl);
    return m_impl->add_event(::std::move(h), ops);
}

void iouring_event_loop::
quick_stop()
{
    if (m_impl)
        m_impl->stop();
}

::std::chrono::milliseconds 
iouring_event_loop::
max_sleep_duration(const per_consumer_attr& attr) const 
{
    if (m_cleaning.load(::std::memory_order_acquire)) [[unlikely]] return 10000ms;
    toolpex_assert(m_impl);
    return ::std::min(200ms, m_impl->max_sleep_duration());
}

void iouring_event_loop::print_status() const
{
    toolpex_assert(m_impl);
    m_impl->print_status();
}

KOIOS_NAMESPACE_END
