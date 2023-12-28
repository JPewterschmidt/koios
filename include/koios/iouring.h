#ifndef KOIOS_IOURING_H
#define KOIOS_IOURING_H

#include <mutex>
#include <shared_mutex>
#include <liburing.h>
#include <cassert>
#include <cstdint>
#include <condition_variable>
#include <system_error>
#include <memory>
#include <chrono>
#include <cstdint>
#include <unordered_map>

#include "koios/macros.h"
#include "toolpex/move_only.h"
#include "toolpex/posix_err_thrower.h"
#include "koios/task_on_the_fly.h"
#include "koios/per_consumer_attr.h"

KOIOS_NAMESPACE_BEG

struct ioret
{
    int32_t ret{};
    uint32_t flags{};
};

namespace iel_detials
{
    class ioret_task
    {
    public:
        ioret_task(::std::shared_ptr<ioret> retslot, task_on_the_fly h) noexcept
            : m_ret{ ::std::move(retslot) }, m_task{ ::std::move(h) }
        {
        }

        void set_ret(int32_t ret, uint32_t flags = 0)
        {
            m_ret->ret = ret;
            m_ret->flags = flags;
        }

        void wakeup();

    private:
        ::std::shared_ptr<ioret> m_ret;
        task_on_the_fly m_task;
    };

    class iouring_event_loop_perthr : public toolpex::move_only
    {
    public:
        iouring_event_loop_perthr(unsigned entries = 1024)
        {
            toolpex::pet e{ ::io_uring_queue_init(entries, &m_ring, 0) };
        }

        ~iouring_event_loop_perthr() noexcept
        {
            ::io_uring_queue_exit(&m_ring);
        }

        iouring_event_loop_perthr(iouring_event_loop_perthr&&) noexcept = default;
        iouring_event_loop_perthr& operator=(iouring_event_loop_perthr&&) noexcept = default;

        void do_occured_nonblk() noexcept;

        void add_event(
            task_on_the_fly h, 
            ::std::shared_ptr<ioret> retslot, 
            ::io_uring_sqe sqe
        );
            
        ::std::chrono::milliseconds max_sleep_duration() const;

    private:
        auto get_lk() const { return ::std::unique_lock{ m_lk }; }
        void dealwith_cqe(const ::io_uring_cqe* cqep);

    private:
        ::io_uring m_ring;
        ::std::unordered_map<uint64_t, ioret_task> m_suspended;
        mutable ::std::mutex m_lk;
        uint8_t m_shot_record{};
    };
}

class iouring_event_loop : public toolpex::move_only
{
private:
    auto get_unilk() const { return ::std::unique_lock{ m_impls_lock }; }
    auto get_shrlk() const { return ::std::shared_lock{ m_impls_lock }; }

public:
    iouring_event_loop() = default;
    void thread_specific_preparation(const per_consumer_attr& attr)
    {
        auto unilk = get_unilk();
        m_impls[attr.thread_id] = ::std::make_shared<iel_detials::iouring_event_loop_perthr>();
    }

    void stop() { stop(get_unilk()); }
    void quick_stop();
    void do_occured_nonblk();
    constexpr void until_done() const noexcept {}
    ::std::chrono::milliseconds max_sleep_duration(const per_consumer_attr&) const;   

    void add_event(
        task_on_the_fly h, 
        ::std::shared_ptr<ioret> retslot, 
        ::io_uring_sqe sqe
    );

private:
    void stop(::std::unique_lock<::std::shared_mutex> lk);
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
        ::std::shared_ptr<iel_detials::iouring_event_loop_perthr>
    > m_impls;
    bool m_cleaning{ false };
    mutable ::std::shared_mutex m_impls_lock;
};

KOIOS_NAMESPACE_END

#endif
