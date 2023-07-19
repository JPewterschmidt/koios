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

#include "koios/macros.h"
#include "koios/invocable_queue_wrapper.h"

KOIOS_NAMESPACE_BEG

enum manually_stop_type { };
extern manually_stop_type manually_stop;

class thread_pool
{
public:
    explicit thread_pool(size_t numthr, invocable_queue_wrapper q);

    thread_pool(size_t numthr, invocable_queue_wrapper q, manually_stop_type)
        : thread_pool(numthr, ::std::move(q))
    { 
    }

    ~thread_pool() noexcept;
          
    template<typename F, typename... Args>
    auto enqueue(F&& func, Args&&... args)
    {
        using return_type = typename std::result_of<F(Args...)>::type;
        auto task = ::std::make_shared<::std::packaged_task<return_type()>>(
            ::std::bind(::std::forward<F>(func), ::std::forward<Args>(args)...)
        );
        ::std::future<return_type> result = task->get_future();
        m_tasks.enqueue([task]{ (*task)(); });
        m_cond.notify_one();

        return result;
    }

    template<typename F, typename... Args>
    void enqueue_no_future(F&& func, Args&&... args)
    {
        auto task = ::std::bind(::std::forward<F>(func), ::std::forward<Args>(args)...);
        m_tasks.enqueue([task = ::std::move(task)]{ task(); });
        m_cond.notify_one();
    }
    
    void stop() noexcept;
    void quick_stop() noexcept;

private:
    void consumer(::std::stop_token token) noexcept;
    [[nodiscard]] bool done(::std::stop_token& tk) const noexcept;
    bool need_stop_now() const noexcept;

private:
    ::std::atomic_bool              m_stop_now{ false };
    ::std::stop_source              m_stop_source;
    invocable_queue_wrapper         m_tasks;
    const bool                      m_manully_stop{ true };
    mutable ::std::mutex            m_lock;
    ::std::condition_variable       m_cond;
    ::std::once_flag                m_stop_once_flag;
    ::std::vector<::std::jthread>   m_thrs;
};

KOIOS_NAMESPACE_END

#endif
