/* Koios, A c++ async runtime library.
 * Copyright (C) 2024  Jeremy Pewterschmidt
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

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
#include <unordered_map>

#include "koios/macros.h"
#include "toolpex/move_only.h"
#include "toolpex/posix_err_thrower.h"
#include "koios/task_on_the_fly.h"
#include "koios/per_consumer_attr.h"
#include "koios/iouring_detials.h"
#include "koios/iouring_ioret.h"

KOIOS_NAMESPACE_BEG

namespace iel_detials
{
    class ioret_task
    {
    public:
        ioret_task(::std::shared_ptr<uring::ioret> retslot, 
                   task_on_the_fly h) noexcept
            : m_ret{ ::std::move(retslot) }, 
              m_task{ ::std::move(h) }
        {
        }

        void set_ret(int32_t ret, uint32_t flags = 0)
        {
            m_ret->ret = ret;
            m_ret->flags = flags;
        }

        void wakeup();

    private:
        ::std::shared_ptr<uring::ioret> m_ret;
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
            ::std::shared_ptr<uring::ioret> retslot, 
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
        unsigned m_shot_record{};
    };
}

/*! \brief Sub event loop of `koios::event_loop` which deal with iouring stuff. */
class iouring_event_loop : public toolpex::move_only
{
private:
    auto get_unilk() const { return ::std::unique_lock{ m_impls_lock }; }
    auto get_shrlk() const { return ::std::shared_lock{ m_impls_lock }; }

public:
    iouring_event_loop() = default;

    /*! \brief the initialization function would be called by `koios::event_loop` after being constructed. 
     *  
     *  every thread would own a uring object,
     *  so the initialization phase needs thread specific infomation
     *  provided by `attr`
     *
     *  \param  attr thread specific infomation.
     */
    void thread_specific_preparation(const per_consumer_attr& attr);
    void stop() { stop(get_unilk()); }
    void quick_stop();
    void do_occured_nonblk();
    constexpr void until_done() const noexcept {}

    ::std::chrono::milliseconds 
    max_sleep_duration(const per_consumer_attr&) const;   

    void add_event(
        task_on_the_fly h, 
        ::std::shared_ptr<uring::ioret> retslot, 
        ::io_uring_sqe sqe
    );

private:
    void stop(::std::unique_lock<::std::shared_mutex> lk);
    auto shrlk_and_curthr_ptr();

private:
    ::std::unordered_map<
        ::std::thread::id, 
        ::std::unique_ptr<iel_detials::iouring_event_loop_perthr>
    > m_impls;
    bool m_cleaning{ false };
    mutable ::std::shared_mutex m_impls_lock;
};

KOIOS_NAMESPACE_END

#endif
