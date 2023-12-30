#include "koios/iouring.h"
#include "koios/runtime.h"
#include <cassert>
#include <thread>
#include <chrono>

KOIOS_NAMESPACE_BEG

using namespace ::std::chrono_literals;
using namespace koios::uring;

namespace iel_detials
{
    void iouring_event_loop_perthr::
    dealwith_cqe(const ::io_uring_cqe* cqep)
    {
        assert(cqep);
        
        const uint64_t key = cqep->user_data;
        auto it = m_suspended.find(key);
        assert(it != m_suspended.end());
        auto cb = ::std::move(it->second);
        m_suspended.erase(it);

        cb.set_ret(cqep->res, cqep->flags);
        cb.wakeup();
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

    void iouring_event_loop_perthr::
    add_event(
        task_on_the_fly h, 
        ::std::shared_ptr<ioret> retslot, 
        ::io_uring_sqe sqe)
    {
        const uint64_t addr = reinterpret_cast<uint64_t>(h.address());
        sqe.user_data = addr;
        auto lk = get_lk();
        ::io_uring_sqe* sqep = ::io_uring_get_sqe(&m_ring);
        m_suspended.insert({
            addr, 
            ioret_task(
                ::std::move(retslot), 
                ::std::move(h)
            )
        });
        *sqep = sqe;
        ::io_uring_submit(&m_ring);
    }

    void ioret_task::
    wakeup()
    {
        get_task_scheduler().enqueue(::std::move(m_task));
    }

    ::std::chrono::milliseconds 
    iouring_event_loop_perthr::
    max_sleep_duration() const
    {
        constexpr uint8_t mask = 2u;
        auto lk = get_lk();
        return static_cast<int>(mask & m_shot_record) * 50ms;
    }
}

void iouring_event_loop::
thread_specific_preparation(const per_consumer_attr& attr)
{
    auto unilk = get_unilk();
    m_impls.insert({
        attr.thread_id, 
        ::std::make_shared<
            iel_detials::iouring_event_loop_perthr
        >()
    });
}

auto iouring_event_loop::
shrlk_and_curthr_ptr()
{
    const auto id = ::std::this_thread::get_id();
    auto lk = get_shrlk();
    assert(m_impls.contains(id));
    return ::std::make_pair(::std::move(lk), m_impls[id]);
}

void iouring_event_loop::
do_occured_nonblk()
{
    auto [lk, ptr] = shrlk_and_curthr_ptr();
    ptr->do_occured_nonblk();
}

void iouring_event_loop::
add_event(
    task_on_the_fly h, 
    ::std::shared_ptr<ioret> retslot, 
    ::io_uring_sqe sqe)
{
    const auto tid = ::std::this_thread::get_id();
    auto lk = get_shrlk();
    if (auto it = m_impls.find(tid); it != m_impls.end())
    {
        it->second->add_event(
            ::std::move(h), ::std::move(retslot), ::std::move(sqe)
        );
    }
    else
    {
        m_impls.begin()->second->add_event(
            ::std::move(h), ::std::move(retslot), ::std::move(sqe)
        );
    }
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
    if (auto it = m_impls.find(attr.thread_id); it != m_impls.end())
        return ::std::min(200ms, it->second->max_sleep_duration());
    return {};
}

KOIOS_NAMESPACE_END
