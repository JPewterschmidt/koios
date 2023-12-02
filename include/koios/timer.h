#ifndef KOIOS_TIMER_H
#define KOIOS_TIMER_H

#include <chrono>
#include <memory>
#include <vector>
#include <utility>
#include <compare>
#include <mutex>
#include <cassert>

#include "koios/macros.h"
#include "koios/task_on_the_fly.h"
#include "koios/task_concepts.h"
#include "koios/exceptions.h"
#include "toolpex/concepts_and_traits.h"

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
    void add_event(toolpex::is_std_chrono_duration auto dura, task_on_the_fly f) noexcept
    {
        const auto now = ::std::chrono::high_resolution_clock::now();
        add_event_impl({ now + dura, ::std::move(f) });
    }

    /*! \brief Adding a timer event.
     *  \param dura The expire time duration.
     *  \param f The callback coroutine when expired.
     */
    void add_event(toolpex::is_std_chrono_duration auto dura, task_concept auto t) noexcept
    {
        auto to_tof = [](auto t){ 
            task_on_the_fly result = t;
            return result;
        };
        add_event(dura, to_tof(::std::move(t)));
    }

    /*! \brief returning the maximum sleep duration of the `thread_pool`
     *         which acceptable to timer event loop to ensure
     *         the timer accuracy.
     */
    auto max_sleep_duration() noexcept
    {
        const auto now = ::std::chrono::high_resolution_clock::now();
        ::std::unique_lock lk{ m_lk };
        if (m_timer_heap.empty()) return ::std::chrono::nanoseconds::max();
        return m_timer_heap.front().timeout_tp - now;
    }
    
    void quick_stop() noexcept { m_timer_heap.clear(); }

    // This function should be called by th eventloop during it's
    // destruction phase which the event_loop would prevent further 
    // `add_event` operation.
    [[nodiscard]] bool done() const noexcept
    {
        ::std::unique_lock lk{ m_lk };
        return m_timer_heap.empty();
    }

private:
    void add_event_impl(timer_event te) noexcept;

private:
    ::std::vector<timer_event> m_timer_heap;
    mutable ::std::mutex m_lk;
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
    timer_event_loop()
        : m_impl_ptr { ::std::make_shared<timer_event_loop_impl>() }
    {
    }

    void do_occured_nonblk() noexcept 
    { 
        m_impl_ptr->do_occured_nonblk(); 
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
        m_impl_ptr->add_event(::std::forward<Args>(args)...); 
    }

    ::std::chrono::nanoseconds max_sleep_duration() noexcept
    { 
        return m_impl_ptr->max_sleep_duration(); 
        return ::std::chrono::nanoseconds::max();
    }

    void quick_stop() noexcept;
    void stop();

    /*! \attention should only be called by `event_loop`.*/
    void until_done();

    constexpr void thread_specific_preparation() noexcept { }

    bool is_cleanning() const;
    bool done() { return m_impl_ptr->done(); }   

private:    
    ::std::atomic_bool m_cleanning{ false };
    ::std::shared_ptr<timer_event_loop_impl> m_impl_ptr;
};

KOIOS_NAMESPACE_END

#endif
