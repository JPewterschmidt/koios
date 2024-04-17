#include <chrono>
#include <fstream>
#include "toolpex/exceptions.h"
#include "koios/dir_mutex.h"
#include "koios/iouring_awaitables.h"
#include "koios/this_task.h"

namespace koios
{

namespace fs = ::std::filesystem;

void dir_mutex_guard::unlock() noexcept
{
    if (m_parent) 
    {
        m_parent->unlock();
        m_parent = nullptr;
    }
}

dir_mutex_guard::dir_mutex_guard(dir_mutex_guard&& other) noexcept
    : m_parent{ ::std::exchange(other.m_parent, nullptr) }
{
    toolpex::not_implemented();
}

dir_mutex_guard& dir_mutex_guard::operator=(dir_mutex_guard&& other) noexcept
{
    unlock();
    m_parent = ::std::exchange(other.m_parent, nullptr);
    return *this;
}

bool dir_mutex_acq_aw::await_ready() const
{
    return m_parent->hold_this_immediately();
}

void dir_mutex_acq_aw::await_suspend(task_on_the_fly t) noexcept
{
    m_parent->add_awaiting(::std::move(t));
}

dir_mutex_guard dir_mutex_acq_aw::await_resume() noexcept
{
    return { m_parent };   
}

dir_mutex::dir_mutex(::std::filesystem::path p)
    : m_path{ ::std::move(p) }
{
    ::std::error_code ec;
    if (!dir_exists(ec) || ec)
    {
        throw koios::exception{ec};
    }
}

bool dir_mutex::dir_exists(::std::error_code& ec) const noexcept
{
    ec = {};
    return (fs::is_directory(path(), ec) || ec);
}

bool dir_mutex::already_a_lockfile_be() const
{
    ::std::error_code ec{};
    if (!dir_exists(ec) || ec)
    {
        throw koios::exception{ec};
    }
    return fs::exists(path()/lock_file_name(), ec);
}

task<> dir_mutex::polling_lock_file(::std::stop_token tk)
{
    using namespace ::std::chrono_literals;
    co_await this_task::sleep_for(50ms);

    for ( ;; ) 
    {
        if (tk.stop_requested())
            co_return;

        if (already_a_lockfile_be())
            co_await this_task::sleep_for(50ms);
        else
        {
            this->may_wake_next();
            co_return;
        }
    }
}

void dir_mutex::cancel_all_polling() noexcept
{
    m_stop_src.request_stop();
    for (auto& item : m_pollers)
    {
        item.get();
    }
}

dir_mutex::~dir_mutex() noexcept
{
    cancel_all_polling();
    unlock();
}
    
bool dir_mutex::hold_this_immediately()
{
    if (already_a_lockfile_be()) 
    {
        m_pollers.emplace_back(
            polling_lock_file(m_stop_src.get_token())
                .run_and_get_future()
        );
        return false;
    }
    
    bool expected{false};
    const bool success = m_holded.compare_exchange_strong(
        expected, true, 
        ::std::memory_order_release, 
        ::std::memory_order_relaxed);

    if (success)
    {
        ::std::ofstream{path()/lock_file_name()};
    }

    return success;
}

static task<> delete_lock_file(const auto& path)
{
    co_await uring::unlink(path);
}

void dir_mutex::unlock() noexcept
{
    if (!this->may_wake_next()) 
    {
        m_holded.store(false, ::std::memory_order_acquire);
        delete_lock_file(path()).result();
    }
}

} // namespace koios
