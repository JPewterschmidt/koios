#include "koios/iouring.h"
#include <cassert>

KOIOS_NAMESPACE_BEG

namespace iel_detials
{
    static void dealwith_cqe(const ::io_uring_cqe* cqep)
    {
        assert(cqep);
        
        auto it = m_suspended.find(cqep->user_data);
        assert(it != m_suspended.end());
        auto cb = ::std::move(*it);
        m_suspended.erase(it);
        lk.unlock();

        cb.set_ret(cqep->res, cqep->flags);
        cb.wakeup();
    }
    
    void iouring_event_loop_perthr::
    do_occured_nonblk() noexcept
    {
        ::io_uring_cqe* cqep{};
        auto lk = get_lk();

        const size_t left = ::io_uring_cq_ready(&m_ring);
        for (size_t i{}; i < left; ++i)
        {
            int e = ::io_uring_peek_cqe(&m_ring, &cqep);
            if (e == - EAGAIN) // no any CQE available
                break;
            dealwith_cqe(cqep);
        }
    }
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
    const uint64_t addr = reinterpret_cast<uint64_t>(h.address());
    auto lk = get_lk();
    ::io_uring_sqe* sqep = ::io_uring_get_sqe(&m_ring);
    m_suspended[addr] = ioret_task{ ::std::move(retslot), ::std::move(h) };
    *sqep = sqe;
    ::io_uring_submit(&m_ring);
}

void io_uring_event_loop::
stop(::std::unique_ptr<::std::mutex> lk)
{
    m_cleaning = true;
}

void io_uring_event_loop::
quick_stop()
{
    auto lk = get_unilk();
    decltype(m_impls) going_to_die;
    m_suspended.swap(going_to_die);
    stop(::std::move(it));
}

void io_uring_event_loop::
until_done()
{
    auto lk = get_unilk();
    assert(m_cleaning);
    m_stop_cond.wait(lk, [this]{ return m_impls.empty(); });
}

KOIOS_NAMESPACE_END
