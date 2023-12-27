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

#include "koios/macros.h"
#include "koios/invocable_queue_wrapper.h"
#include "koios/exceptions.h"
#include "koios/per_consumer_attr.h" 
#include "koios/queue_concepts.h"

KOIOS_NAMESPACE_BEG

enum manually_stop_type { };
extern manually_stop_type manually_stop;

/*! \brief The most fundamental native thread level management.
 *
 *  You could chouse the underlying queue which contains the invocable objects.
 *
 *  \attention Remeber to call `start()` after initialization !
 */
class thread_pool
{
public:
    /*! \param numthr The number of threads this `thread_pool` hold.
     *  \param q The underlying queue type.
     *
     *  This constructor will mark this `thread_pool` to call the `quick_stop()` in the destructor.
     */
    thread_pool(size_t numthr, invocable_queue_wrapper q)
        : m_tasks{ ::std::move(q) }, 
          m_manully_stop{ false }, 
          m_num_thrs{ numthr },
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
    thread_pool(size_t numthr, invocable_queue_wrapper q, manually_stop_type)
        : thread_pool(numthr, ::std::move(q))
    { 
        m_manully_stop = true;
    }

    /*! Make all the consumer working.
     *  You HAVE TO call this function after initialization.
     */
    void start();

    virtual ~thread_pool() noexcept;
          
    /*! \brief Bind the first and the rest of parameters into a invocable object, the run it on a thread.
     *  \param func The functor.
     *  \param args The arguments of the functor.
     *  \return The corresponding future type, which user could retrive the actuall return value of the enqueued functor.
     *  \see `std::future`
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

    template<typename F, typename... Args>
    [[nodiscard]] auto enqueue(const per_consumer_attr& ca, F&& func, Args&&... args)
    {
        if (need_stop_now()) [[unlikely]] 
            throw thread_pool_stopped_exception{};
        return enqueue_without_checking(ca, ::std::forward<F>(func), ::std::forward<Args>(args)...);
    }

    template<typename F, typename... Args>
    void enqueue_no_future(const per_consumer_attr& ca, F&& func, Args&&... args)
    {
        if (need_stop_now()) [[unlikely]] 
            throw thread_pool_stopped_exception{};
        enqueue_no_future_without_checking(ca, ::std::forward<F>(func), ::std::forward<Args>(args)...);
    }

    /*! \brief Wake up all sleeping threads and join all threads.
     * 
     *  The awakened thread will not go to sleep again, 
     *  and will continue to get functors from the queue. 
     *  Exit the thread until no functors are available.
     */
    void stop() noexcept;

    /*! \brief Wake up all sleeping threads and return immediately.
     * 
     *  Similar to `stop()`, but the holding thread will exit immediately 
     *  without attempting to execute remaining functors in the queue. 
     *  Ownership of the remaining functors goes to the queue object. 
     *  Queue objects are responsible for destroying them.
     */
    void quick_stop() noexcept;

    /*! \return the number of remain tasks.
     */
    size_t number_remain_tasks() const noexcept { return m_tasks.size(); }

    /*! \return the number of threads this pool managing.
     */
    size_t number_of_threads() const noexcept { return m_thrs.size(); }

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
        m_cond.notify_all();
    }

    template<typename F, typename... Args>
    void enqueue_no_future_without_checking(const per_consumer_attr& ca, F&& func, Args&&... args) noexcept
    {
        auto task = ::std::bind(::std::forward<F>(func), ::std::forward<Args>(args)...);
        m_tasks.enqueue(ca, [task = ::std::move(task)] mutable { task(); });
        m_cond.notify_all();
    }

    template<typename F, typename... Args>
    [[nodiscard]] auto enqueue_without_checking(F&& func, Args&&... args) noexcept
    {
        // Dear developers: 
        // Do NOT try to throw anything in this function.
        // See the comments of `enqueue_no_future`.

        using return_type = typename std::result_of<F(Args...)>::type;
        using future_type = ::std::future<return_type>;

        auto task = ::std::make_shared<::std::packaged_task<return_type()>>(
            ::std::bind(::std::forward<F>(func), ::std::forward<Args>(args)...)
        );
        future_type result = task->get_future();
        m_tasks.enqueue([task] mutable { (*task)(); });
        m_cond.notify_all();

        return result;
    }

    template<typename F, typename... Args>
    [[nodiscard]] auto enqueue_without_checking(const per_consumer_attr& ca, F&& func, Args&&... args) noexcept
    {
        // Dear developers: 
        // Do NOT try to throw anything in this function.
        // See the comments of `enqueue_no_future`.

        using return_type = typename std::result_of<F(Args...)>::type;
        using future_type = ::std::future<return_type>;

        auto task = ::std::make_shared<::std::packaged_task<return_type()>>(
            ::std::bind(::std::forward<F>(func), ::std::forward<Args>(args)...)
        );
        future_type result = task->get_future();
        m_tasks.enqueue(ca, [task] mutable { (*task)(); });
        m_cond.notify_all();

        return result;
    }

    virtual void thread_specific_preparation(const per_consumer_attr& attr)
    {
        m_tasks.thread_specific_preparation(attr);
    }

    virtual void before_each_task() noexcept { }
    virtual ::std::chrono::nanoseconds max_sleep_duration() noexcept 
    { 
        return ::std::chrono::nanoseconds::max(); 
    }

    bool need_stop_now() const noexcept;

private:
    void consumer(::std::stop_token token, const ::std::thread::id main_thread_id) noexcept;
    [[nodiscard]] bool done(::std::stop_token& tk) const noexcept;

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
    ::std::latch                    m_start_working;
};

KOIOS_NAMESPACE_END

#endif
