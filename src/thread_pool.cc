#include "koios/thread_pool.h"

KOIOS_NAMESPACE_BEG

manually_stop_type manually_stop{};

thread_pool::thread_pool(size_t numthr)
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

thread_pool::thread_pool(size_t numthr, manually_stop_type)
    : thread_pool(numthr)
{
    m_manully_stop = true;
}

thread_pool::~thread_pool() noexcept
{
    if (m_manully_stop) return;
    quick_stop();
}

void thread_pool::stop() noexcept
{
    if (m_numthrs == 0) return;
    m_stop_source.request_stop();
    m_cond.notify_all();

    for (auto& thr : m_thrs)
    {
        if (thr.joinable()) thr.join();
    }
}

void thread_pool::quick_stop() noexcept
{
    if (m_numthrs == 0) return;
    m_stop_now.store(true, ::std::memory_order::release); // TODO
    stop();
}

void thread_pool::consumer(::std::stop_token token) noexcept
{
    do
    {
        ::std::function<void()> task;

        if (auto task_opt = m_tasks.dequeue(); !task_opt)
        {
            ::std::unique_lock lk{ m_lock };
            m_cond.wait(lk);
        }
        else task = ::std::move(task_opt.value());

        if (!task) [[unlikely]] continue;

        try { task(); } catch (...) {}
    }
    while (!done(token));
}

bool thread_pool::done(::std::stop_token& tk) const noexcept
{
    if (!tk.stop_requested())
        return false;

    if (need_stop_now()) return true;
    return m_tasks.empty();
}

bool thread_pool::need_stop_now() const noexcept
{
    return m_stop_now.load(::std::memory_order::acquire); // TODO
}

KOIOS_NAMESPACE_END
