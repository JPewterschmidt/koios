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

#ifndef KOIOS_TIMER_H
#define KOIOS_TIMER_H

#include <chrono>
#include <memory>
#include <vector>
#include <utility>
#include <compare>
#include <mutex>
#include <thread>
#include <shared_mutex>
#include <stop_token>
#include <latch>

#include "koios/macros.h"
#include "koios/task_on_the_fly.h"
#include "koios/task_concepts.h"
#include "koios/exceptions.h"
#include "koios/per_consumer_attr.h"
#include "toolpex/concepts_and_traits.h"
#include "toolpex/exceptions.h"

KOIOS_NAMESPACE_BEG

/*! \brief timepoint and callback aggregate object stored in the MinHeap
 *  Only used by `class timer_event_loop_impl`
 */
struct timer_event
{
    ::std::chrono::time_point<::std::chrono::high_resolution_clock> timeout_tp;
    task_on_the_fly task;

    /*! \brief Forward this compration to the 3-way compration
     *         of `std::chrono::duration`.
     */
    friend ::std::strong_ordering 
    operator<=>(const timer_event& lhs, const timer_event& rhs) noexcept;
};

/*! \brief The implementation class of timer eventloop. 
 *  And it's thread safe.
 *
 *  \see `event_loop`
 */
class timer_event_loop_impl
{
public:
    timer_event_loop_impl(::std::stop_token tk) noexcept
        : m_stop_tk{ ::std::move(tk) } 
    {
    }

    /*! \brief Satisfy the requirement of `event_loop`
     *
     *  It will schedule all the expired time event callback 
     *  coroutine at each calling of this function.
     *  Basically this function would be called by `event_loop`
     */
    void do_occured_nonblk() noexcept;

    /*! \brief Adding a timer event.
     *  \param dura The expire time duration.
     *  \param f The callback coroutine when expired.
     */
    void add_event(toolpex::is_std_chrono_duration auto dura, auto task) noexcept
    {
        const auto now = ::std::chrono::high_resolution_clock::now();
        add_event(now + dura, ::std::move(task));
    }

    /*! \brief Adding a timer event.
     *  \param tp The expire time point.
     *  \param f The callback coroutine when expired.
     */
    void add_event(toolpex::is_std_chrono_time_point auto tp, auto task) noexcept
    {
        auto tof = [](auto&& t)
        {
            task_on_the_fly result = ::std::move(t);
            return result;
        };
        add_event_impl({ tp, tof(::std::move(task)) });
    }

    /*! \brief returning the maximum sleep duration of the `thread_pool`
     *         which acceptable to timer event loop to ensure
     *         the timer accuracy.
     */
    auto max_sleep_duration() const noexcept
    {
        const auto now = ::std::chrono::high_resolution_clock::now();
        ::std::shared_lock lk{ m_lk };
        if (m_timer_heap.empty()) return ::std::chrono::nanoseconds::max();
        return m_timer_heap.front().timeout_tp - now;
    }
    
    void quick_stop() noexcept { m_timer_heap.clear(); }

    // This function should be called by th eventloop during it's
    // destruction phase which the event_loop would prevent further 
    // `add_event` operation.
    [[nodiscard]] bool done() const noexcept
    {
        ::std::shared_lock lk{ m_lk };
        return m_timer_heap.empty();
    }

private:
    void wake_up_all() noexcept;

private:
    void add_event_impl(timer_event te) noexcept;

private:
    ::std::vector<timer_event> m_timer_heap;
    mutable ::std::shared_mutex m_lk;
    ::std::stop_token m_stop_tk;
};

/*! \brief A wrap class of timer event.
 *  
 *  This class is used to implement per thread timer,
 *  which would increase the performance.
 *
 *  \todo Implement the per-thread timer.
 */
class timer_event_loop
{
public:
    timer_event_loop() = default;

    void do_occured_nonblk() noexcept 
    { 
        auto [lk, ptr] = cur_thread_ptr();
        ptr->do_occured_nonblk(); 
    }

    template<typename... Args>
    void add_event(Args&&... args) noexcept 
    { 
        if (is_cleanning()) 
        {
            koios::log_error(
                "event loop has started cleaning! "
                "you can't add event any more."
            );
            return;
        }
        
        auto [lk, ptr] = cur_thread_ptr();
        ptr->add_event(::std::forward<Args>(args)...); 
    }

    ::std::chrono::nanoseconds
    max_sleep_duration(const per_consumer_attr& cattr) noexcept
    { 
        if (is_cleanning()) 
            return ::std::chrono::nanoseconds::max();
        auto lk = ::std::shared_lock{ m_ptrs_lock };
        return m_impl_ptrs[cattr.thread_id]->max_sleep_duration();
    } 

    void quick_stop() noexcept;
    void stop() noexcept { m_stop_src.request_stop(); }

    /*! \attention should only be called by `event_loop`.*/
    constexpr void until_done() const noexcept {}

    void thread_specific_preparation(const per_consumer_attr& attr)  
    { 
        ::std::unique_lock lk{ m_ptrs_lock };
        m_impl_ptrs.insert({
            attr.thread_id, 
            ::std::make_unique<timer_event_loop_impl>(
                m_stop_src.get_token() 
            )
        });
    }

    bool is_cleanning() const { return m_stop_src.stop_requested(); }
    bool done();

private:
    ::std::pair<
        ::std::shared_lock<::std::shared_mutex>, 
        timer_event_loop_impl*> 
    cur_thread_ptr()
    {
        ::std::shared_lock lk{ m_ptrs_lock };
        auto id = ::std::this_thread::get_id();
        auto* ptr = m_impl_ptrs[id].get();
        if (!m_impl_ptrs.contains(id))
        {
            ptr = m_impl_ptrs.begin()->second.get();
        }
        return { ::std::move(lk), ptr };
    }

private:    
    ::std::unordered_map<
        ::std::thread::id, 
        ::std::unique_ptr<timer_event_loop_impl>
    > m_impl_ptrs;
    mutable ::std::shared_mutex m_ptrs_lock;
    ::std::stop_source m_stop_src;
};

KOIOS_NAMESPACE_END

#endif
