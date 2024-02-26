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

#include <atomic>
#include <shared_mutex>
#include <liburing.h>
#include <cassert>
#include <cstdint>
#include <condition_variable>
#include <system_error>
#include <memory>
#include <chrono>
#include <unordered_map>
#include <utility>

#include "koios/macros.h"
#include "toolpex/move_only.h"
#include "toolpex/posix_err_thrower.h"
#include "toolpex/spin_lock.h"
#include "koios/task_on_the_fly.h"
#include "koios/task_release_once.h"
#include "koios/per_consumer_attr.h"
#include "koios/iouring_ioret.h"
#include "koios/iouring_op_batch_rep.h"

KOIOS_NAMESPACE_BEG

namespace iel_detials
{
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

    public:
        ::std::shared_ptr<task_release_once> 
        add_event(task_on_the_fly h, 
                  ::std::shared_ptr<uring::ioret> retslot, 
                  ::io_uring_sqe sqe);

        void add_event(task_on_the_fly h, uring::op_batch_rep& ops); 

        ::std::chrono::milliseconds max_sleep_duration() const;

    private:
        auto get_lk() const { return ::std::unique_lock{ m_lk }; }
        void dealwith_cqe(const ::io_uring_cqe* cqep);
        size_t mis_shot_indicator() const noexcept { return m_num_mis_shot - 1; }
        void mis_shot_this_time() noexcept { m_num_mis_shot <<= 1; }
        void shot_this_time() noexcept { m_num_mis_shot = 1; }

    private:
        ::io_uring m_ring;
        ::std::unordered_map<
            uint64_t, 
            ::std::pair<uring::op_batch_rep*, task_on_the_fly>
        > m_opreps;
        mutable toolpex::spin_lock m_lk;
        ::std::size_t m_num_mis_shot{1};
    };
}

/*! \brief Sub event loop of `koios::event_loop` which deal with iouring stuff. 
 *
 *  This class is a wrapper.
 *  Every thread will has there own `iouring_event_loop_perthr` as the iouring related event_loop.
 *  and any single `iouring_event_loop_perthr` represents a single iouring object.
 */
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

    /*! \brief A blockable function after it's return 
     *         means the top level event_loop could exit safely.
     *
     *  Actually do nothing, just for satisfy the requirement of `event_loop`.
     */
    constexpr void until_done() const noexcept {}

    /*! \brief  Get the maximum time gap 
     *          between the last `do_occured_nonblk()` and the next.
     *
     *  This value will be evaluated fuzzly due to multithreading performance concern.
     */
    ::std::chrono::milliseconds 
    max_sleep_duration(const per_consumer_attr&) const;   

    /*! \brief  Inform the OS, you want do some IO operations, 
     *          and register the task handler to the internal container.
     *
     *  \param h The handler of task issued a iouring operation(s).
     *  \parma ops Represents those operations.
     */
    void add_event(task_on_the_fly h, uring::op_batch_rep& ops); 

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
