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

#ifndef KOIOS_THREAD_POOL_H
#define KOIOS_THREAD_POOL_H

#include <thread>
#include <vector>
#include <memory>
#include <future>
#include <utility>
#include <mutex>
#include <stop_token>
#include <functional>
#include <condition_variable>
#include <latch>
#include <chrono>

#include "koios/env.h"
#include "koios/macros.h"
#include "koios/invocable_queue_wrapper.h"
#include "koios/exceptions.h"
#include "koios/per_consumer_attr.h" 
#include "koios/queue_concepts.h"
#include "toolpex/move_only.h"

KOIOS_NAMESPACE_BEG

enum manually_stop_type { };
extern manually_stop_type manually_stop;

/*! \brief The most fundamental native thread level management.
 *
 *  You could chouse the underlying queue which contains the invocable objects.
 *
 *  \attention Remeber to call `start()` after initialization !
 */
class thread_pool : public toolpex::move_only
{
public:
    /*! \param numthr The number of threads this `thread_pool` hold.
     *  \param q The underlying queue type.
     *
     *  This constructor will mark this `thread_pool` to call the `quick_stop()` in the destructor.
     */
    thread_pool(size_t numthr, invocable_queue_wrapper q, size_t queue_capa_hint = 65536)
        : m_tasks{ ::std::move(q) }, 
          m_manully_stop{ false }, 
          m_num_thrs{ numthr },
          m_queue_capa_hint{ queue_capa_hint },
          m_start_working{ static_cast<long int>(numthr) }
    {
    }

    /*! \param numthr The number of threads this `thread_pool` hold.
     *  \param q The underlying queue type.
     *
     *  This constructor will mark this `thread_pool` to NOT call the `quick_stop()` in the destructor.
     *  If you want to stop the `thread_pool`, you need call the `stop()` or `quick_stop()` manually.
     *  Or the destructor will blocked.
     */
    thread_pool(size_t numthr, invocable_queue_wrapper q, manually_stop_type, size_t queue_capa_hint = 65536)
        : thread_pool(numthr, ::std::move(q), queue_capa_hint)
    { 
        m_manully_stop = true;
    }

    /*! Make all the consumer working.
     *  You HAVE TO call this function after initialization.
     */
    void start()
    {
        const auto main_thread_id = ::std::this_thread::get_id();
        for (size_t i = 0; i < m_num_thrs; ++i)
        {
            m_thrs.emplace_back([this, main_thread_id]() noexcept { 
                this->consumer(m_stop_source.get_token(), main_thread_id); 
            });
        }
        m_start_working.wait();
    }

    virtual ~thread_pool() noexcept
    {
        if (m_manully_stop) return;
        quick_stop();
    }
          
    /*! \brief Bind the first and the rest of parameters into a invocable object, the run it on a thread.
     *  \param func The functor.
     *  \param args The arguments of the functor.
     *  \return The corresponding future type, which user could retrive the actuall return value of the enqueued functor.
     *  \see `koios::future`
     *  \see `enqueue_no_future()`
     *
     *  Lower than `enqueue_no_future()`
     */
    template<typename F, typename... Args>
    [[nodiscard]] auto enqueue(F&& func, Args&&... args)
    {
        if (need_stop_now()) [[unlikely]] 
            throw thread_pool_stopped_exception{};
        return enqueue_without_checking(::std::forward<F>(func), ::std::forward<Args>(args)...);
    }

    /*! \brief Bind the first and the rest of parameters into a invocable object, the run it on a thread.
     *  \param func The functor.
     *  \param args The arguments of the functor.
     *  \see `enqueue()`
     *
     *  Faster than `enqueue()`
     */
    template<typename F, typename... Args>
    void enqueue_no_future(F&& func, Args&&... args)
    {
        if (need_stop_now()) [[unlikely]] 
            throw thread_pool_stopped_exception{};
        enqueue_no_future_without_checking(::std::forward<F>(func), ::std::forward<Args>(args)...);
    }

    /*! \brief  Basically same as `enqueue` without `ca` parameter.
     *  \param  ca The consumer attribute object reference.
     *
     *  This function would schedule `func` to the thread specified by `ca`
     */
    template<typename F, typename... Args>
    [[nodiscard]] auto enqueue(const per_consumer_attr& ca, F&& func, Args&&... args)
    {
        if (need_stop_now()) [[unlikely]] 
            throw thread_pool_stopped_exception{};
        return enqueue_without_checking(ca, ::std::forward<F>(func), ::std::forward<Args>(args)...);
    }

    /*! \brief  Basically same as `enqueue_no_future` without `ca` parameter.
     *  \param  ca The consumer attribute object reference.
     *
     *  This function would schedule `func` to the thread specified by `ca`
     */
    template<typename F, typename... Args>
    void enqueue_no_future(const per_consumer_attr& ca, F&& func, Args&&... args)
    {
        if (need_stop_now()) [[unlikely]] 
            throw thread_pool_stopped_exception{};
        enqueue_no_future_without_checking(ca, ::std::forward<F>(func), ::std::forward<Args>(args)...);
    }

    void wake_up() noexcept
    {
        m_cond.notify_one();
    }

    bool is_cleaning() const noexcept
    {
        return m_stop_now.load();
    }

    /*! \brief Wake up all sleeping threads and join all threads.
     * 
     *  The awakened thread will not go to sleep again, 
     *  and will continue to get functors from the queue. 
     *  Exit the thread until no functors are available.
     */
    void stop() noexcept
    {
        ::std::call_once(m_stop_once_flag, [this]{
            m_stop_source.request_stop();
            m_cond.notify_all();

            for (auto& thr : m_thrs)
            {
                m_cond.notify_all();
                if (thr.joinable()) thr.join();
            }
        });
    }


    /*! \brief Wake up all sleeping threads and return immediately.
     * 
     *  Similar to `stop()`, but the holding thread will exit immediately 
     *  without attempting to execute remaining functors in the queue. 
     *  Ownership of the remaining functors goes to the queue object. 
     *  Queue objects are responsible for destroying them.
     */
    void quick_stop() noexcept
    {
        m_stop_now.store(true, ::std::memory_order::release);
        stop();
    }

    /*! \return the number of remain tasks.
     */
    size_t number_remain_tasks() const noexcept { return m_tasks.size(); }

    /*! \return the number of threads this pool managing.
     */
    size_t number_of_threads() const noexcept { return m_thrs.size(); }

    /*! \return the colletions of pointers of each attribute of consumer thread. */
    const ::std::vector<const per_consumer_attr*>& consumer_attrs() noexcept
    {
        return m_consumer_attrs;
    }

protected:
    template<typename F, typename... Args>
    void enqueue_no_future_without_checking(F&& func, Args&&... args) noexcept
    {
        // Dear developers: 
        // Do NOT try to throw anything in this function.
        // Because one of the most important caller of this function, `suspend_await`, 
        // will take the ownership (by holding a `task_on_the_fly` instance ) of the coroutine handler of its caller.
        // Throwing something will trigger the destuctor of that `task_on_the_fly` instance.
        // Which means the resources of that caller will also be destructed!
        // But after that, something you thrown here will continue propogating,
        // which will cause problem like *use-after-free* or *double-free*.

        auto task = ::std::bind(::std::forward<F>(func), ::std::forward<Args>(args)...);
        m_tasks.enqueue([task = ::std::move(task)] mutable { task(); });
        wake_up();
    }

    template<typename F, typename... Args>
    void enqueue_no_future_without_checking(const per_consumer_attr& ca, F&& func, Args&&... args) noexcept
    {
        auto task = ::std::bind(::std::forward<F>(func), ::std::forward<Args>(args)...);
        m_tasks.enqueue(ca, [task = ::std::move(task)] mutable { task(); });
        wake_up();
    }

    template<typename F, typename... Args>
    [[nodiscard]] auto enqueue_without_checking(F&& func, Args&&... args) noexcept
    {
        // Dear developers: 
        // Do NOT try to throw anything in this function.
        // See the comments of `enqueue_no_future`.

        using return_type = typename std::result_of<F(Args...)>::type;

        auto task = ::std::make_shared<::std::packaged_task<return_type()>>(
            ::std::bind(::std::forward<F>(func), ::std::forward<Args>(args)...)
        );
        auto result = task->get_future();
        m_tasks.enqueue([task] mutable { (*task)(); });
        wake_up();

        return result;
    }

    template<typename F, typename... Args>
    [[nodiscard]] auto enqueue_without_checking(const per_consumer_attr& ca, F&& func, Args&&... args) noexcept
    {
        // Dear developers: 
        // Do NOT try to throw anything in this function.
        // See the comments of `enqueue_no_future`.

        using return_type = typename std::result_of<F(Args...)>::type;

        auto task = ::std::make_shared<::std::packaged_task<return_type()>>(
            ::std::bind(::std::forward<F>(func), ::std::forward<Args>(args)...)
        );
        auto result = task->get_future();
        m_tasks.enqueue(ca, [task] mutable { (*task)(); });
        wake_up();

        return result;
    }

    virtual void thread_specific_preparation(const per_consumer_attr& attr)
    {
        m_tasks.thread_specific_preparation(attr);
        ::std::lock_guard lk{ m_lock };
        m_consumer_attrs.push_back(&attr);
    }

    virtual void before_each_task() noexcept { }
    virtual ::std::chrono::nanoseconds max_sleep_duration([[maybe_unused]] const per_consumer_attr&) noexcept 
    { 
        return ::std::chrono::nanoseconds::max(); 
    }

    bool need_stop_now() const noexcept
    {
        return m_stop_now.load(::std::memory_order_acquire);
    }

private:
    void consumer(::std::stop_token         token, 
                  const ::std::thread::id   main_thread_id) noexcept
    {
        const per_consumer_attr cattr{ 
            .thread_id = ::std::this_thread::get_id(),
            .main_thread_id = main_thread_id,
            .number_of_threads = number_of_threads(),
        };
        thread_specific_preparation(cattr);
        if (::std::this_thread::get_id() != main_thread_id)
            m_start_working.count_down();
        while (!done(token))
        {
            before_each_task();
            if (auto task_opt = m_tasks.dequeue(cattr); !task_opt)
            {
                if (done(token)) break;
                ::std::unique_lock lk{ m_lock };
                const auto max_waiting_time = max_sleep_duration(cattr);
                constexpr auto waiting_latch = is_profiling_mode() ? ::std::chrono::milliseconds{1} : ::std::chrono::milliseconds{100};
                const auto actual_waiting_time = 
                    waiting_latch < max_waiting_time ? waiting_latch : max_waiting_time;

                m_cond.wait_for(lk, actual_waiting_time);
            }
            else try 
            { 
                task_opt.value()(); 
            } 
            catch (const koios::exception& e)
            {
                e.log();
            }
            catch (const ::std::exception& e)
            {
                koios::log_error(e.what());
            }
            catch (...)
            { 
                koios::log_error("user code has throw something not inherited from `std::exception`");
            }
        }
    }

    [[nodiscard]] bool done(::std::stop_token& tk) const noexcept
    {
        if (!tk.stop_requested())
            return false;

        if (need_stop_now()) return true;
        return m_tasks.empty();
    }


private:
    ::std::atomic_bool              m_stop_now{ false };
    ::std::stop_source              m_stop_source;
    invocable_queue_wrapper         m_tasks;
    bool                            m_manully_stop{ true };
    mutable ::std::mutex            m_lock;
    ::std::condition_variable       m_cond;
    ::std::once_flag                m_stop_once_flag;
    ::std::vector<::std::jthread>   m_thrs;
    size_t                          m_num_thrs{};
    size_t                          m_queue_capa_hint{};
    ::std::latch                    m_start_working;
    ::std::vector<const per_consumer_attr*> m_consumer_attrs{};
};

KOIOS_NAMESPACE_END

#endif
