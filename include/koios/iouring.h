#ifndef KOIOS_IOURING_H
#define KOIOS_IOURING_H

#include <unordered_map>
#include <mutex>
#include <shared_mutex>
#include <liburing.h>
#include <cassert>
#include <cstdint>
#include <condition_variable>
#include <error_code>
#include <system_error>

#include "koios/macros.h"
#include "toolpex/move_only.h"
#include "toolpex/posix_err_thrower.h"
#include "koios/runtime.h"

KOIOS_NAMESPACE_BEG

struct ioret
{
    int32_t ret{};
    uint32_t flags{};
};

class ioret_result : public ioret
{
public:
    ioret_result(ioret r)
        : ioret{ ::std::move(r) }
    {
        m_ec = ::std::error_code{
            this->ret, ::std::system_error()
        };
    }

    ioret_result() = default; // TODO

private:
    ::std::error_code m_ec;
};

namespace iel_detials
{
    class ioret_task
    {
    public:
        ioret_task(::std:;shared_ptr<ioret> retslot, task_on_the_fly h) noexcept
            : m_ret{ ::std::move(retslot) }, m_task{ ::std::move(h) }
        {
        }

        void set_ret(int32_t ret, uint32_t flags = 0)
        {
            m_ret->ret = ret;
            m_ret->flags = flags;
        }

        void wakeup()
        {
            get_task_scheduler().enqueue(::std::move(m_task));
        }

    private:
        ::std::shared_ptr<ioret> m_ret;
        task_on_the_fly m_task;
    };

    class iouring_event_loop_perthr : public toolpex::move_only
    {
    public:
        iouring_event_loop_perthr(size_t entries = 1024)
        {
            toolpex::pet e{ ::io_uring_queue_init(entries, &m_ring, 0) };
        }

        ~iouring_event_loop_perthr() noexcept
        {
            ::io_uring_queue_exit(&m_ring);
        }

        void do_occured_nonblk() noexcept;

    private:
        auto get_lk() { return ::std::unique_lock{ m_lk }; }

    private:
        ::io_uring m_ring;
        ::std::unordered_map<uint64_t, ioret_task> m_suspended;
        mutable ::std::mutex m_lk;
    };
}

class iouring_event_loop
{
public:
    void thread_specific_preparation(const per_consumer_attr& attr)
    {
        auto unilk = get_unilk();
        m_impls[attr.thread_id] = iel_detials::iouring_event_loop_perthr{};
    }

    void stop() { stop(get_unilk()); }
    void quick_stop();
    void until_done();
    void do_occured_nonblk();

    void add_event(
        task_on_the_fly h, 
        ::std::shared_ptr<ioret> retslot, 
        ::io_uring_sqe sqe
    );

private:
    void stop(::std::unique_lock<::std::mutex> lk);
    auto get_unilk() { return ::std::unique_lock{ m_impls_lock }; }
    auto get_shrlk() { return ::std::shared_lock{ m_impls_lock }; }
    auto shrlk_and_curthr_ptr()
    {
        const auto id = ::std::this_thread::get_id();
        auto lk = get_shrlk();
        assert(m_impls.contains(id));
        return ::std::make_pair(::std::move(lk), m_impls[id]);
    }

private:
    ::std::unordered_map<
        ::std::thread::id, 
        ::std::shared_ptr<iel_detials::iouring_event_loop_perthr
    > m_impls;
    bool m_cleaning{ false };
    ::std::condition_variable m_stop_cond;
    mutable ::std::shared_mutex m_impls_lock;
};

class iouring_aw
{
public:
    io_uring_aw(::io_uring_sqe sqe) 
        : m_ret{ ::std::make_shared<ioret>() }, 
          m_sqe{ sqe }
    {
    }

    bool await_ready() const noexcept 
    { 
        return !get_task_scheduler().is_cleanning(); // TODO
                                                     // this function call are not implemented yet, 
                                                     // please implement it!
    }

    void await_suspend(task_on_the_fly h) 
    {
        get_task_scheduler().add_event<iouring_event_loop>(
            ::std::move(h), m_ret, m_seq
        );
    }

    ioret_result await_resume() 
    { 
        return { *m_ret };
    }

private:
    ::std::shared_ptr<ioret> m_ret;
    ::io_uring_sqe m_sqe{};
};

KOIOS_NAMESPACE_END

#endif
