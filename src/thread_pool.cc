#include <chrono>
#include "koios/thread_pool.h"
#include "koios/exceptions.h"
#include "spdlog/spdlog.h"

using namespace ::std::chrono_literals;

KOIOS_NAMESPACE_BEG

manually_stop_type manually_stop{};

void thread_pool::start()
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

thread_pool::~thread_pool() noexcept
{
    if (m_manully_stop) return;
    quick_stop();
}

void thread_pool::stop() noexcept
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

void thread_pool::quick_stop() noexcept
{
    m_stop_now.store(true, ::std::memory_order::release); // TODO
    stop();
}

void thread_pool::consumer(
    ::std::stop_token token, 
    const ::std::thread::id mt_id) noexcept
{
    const per_consumer_attr cattr{ 
        .main_thread_id = mt_id,
        .number_of_threads = number_of_threads(),
    };
    thread_specific_preparation(cattr);
    if (::std::this_thread::get_id() != mt_id)
        m_start_working.count_down();
    while (!done(token))
    {
        before_each_task();
        if (auto task_opt = m_tasks.dequeue(cattr); !task_opt)
        {
            if (done(token)) break;
            ::std::unique_lock lk{ m_lock };
            const auto max_waiting_time = max_sleep_duration();
            constexpr auto waiting_latch = 50ms;
            m_cond.wait_for(lk, waiting_latch < max_waiting_time ? waiting_latch : max_waiting_time);
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
