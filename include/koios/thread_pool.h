#ifndef KOIOS_THREAD_POOL_H
#define KOIOS_THREAD_POOL_H

#include <functional>
#include <thread>
#include <vector>
#include <memory>
#include <stop_token>
#include <utility>
#include <future>
#include "koios/macros.h"
#include "concurrentqueue/blockingconcurrentqueue.h"

KOIOS_NAMESPACE_BEG

constexpr void noop_function() noexcept {}

class thread_pool
{
public:
    explicit thread_pool(size_t numthr)
        : m_numthrs{ numthr }, 
          m_active_threads{ numthr }
    {
        for (size_t i = 0; i < numthr; ++i)
        {
            m_thrs.emplace_back([this]() noexcept { 
                this->consumer(m_stop_source.get_token()); 
            });
        }
    }

    ~thread_pool()
    {
        stop();
    }
          
    template<typename F, typename... Args>
    auto enqueue(F&& func, Args&&... args)
    {
        using return_type = typename std::result_of<F(Args...)>::type;
        auto task = ::std::make_shared<::std::packaged_task<return_type()>>(
            ::std::bind(::std::forward<F>(func), ::std::forward<Args>(args)...)
        );
        ::std::future<return_type> result = task->get_future();
        m_tasks.enqueue([task]{ (*task)(); });

        return result;
    }
    
    void stop()
    {
        m_stop_source.request_stop();
        while (m_active_threads.load() > 0)
        {
            m_tasks.enqueue(noop_function);
        }
    }

    auto size() const noexcept { return m_active_threads.load(); }

private:
    void consumer(::std::stop_token token) noexcept
    {
        while (!token.stop_requested())
        {
            ::std::function<void()> task;
            m_tasks.wait_dequeue(task);
            if (!task) [[unlikely]]
                continue;
            try { task(); } catch (...) {}
        }
        m_active_threads.fetch_sub(1u);
    }

private:
    size_t m_numthrs;
    ::std::atomic_size_t m_active_threads;
    ::std::stop_source m_stop_source;
    moodycamel::BlockingConcurrentQueue<::std::function<void()>> m_tasks;
    ::std::vector<::std::jthread> m_thrs;
};

KOIOS_NAMESPACE_END

#endif
