// This file is part of Koios
// https://github.com/JPewterschmidt/koios
//
// Copyleft 2023 - 2024, ShiXin Wang. All wrongs reserved.

#include <chrono>
#include <fstream>
#include <mutex>

#include <sys/stat.h>
#include <fcntl.h>

#include "toolpex/exceptions.h"
#include "toolpex/errret_thrower.h"
#include "toolpex/string_ext.h"

#include "koios/dir_mutex.h"
#include "koios/iouring_awaitables.h"
#include "koios/this_task.h"
#include "koios/task.h"

namespace koios
{

namespace fs = ::std::filesystem;
using namespace ::std::chrono_literals;

static toolpex::errret_thrower et{};

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
}

dir_mutex_guard& dir_mutex_guard::operator=(dir_mutex_guard&& other) noexcept
{
    this->unlock();
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

dir_mutex::dir_mutex(::std::filesystem::path p, 
                     ::std::chrono::milliseconds polling_period)
    : m_path{ ::std::move(p) }, 
      m_dirfd{ et << ::open(dir_path().c_str(), O_DIRECTORY) }, 
      m_polling_period{ polling_period > 10ms ? polling_period : 10ms }
{
    typename ::stat st{};
    et << ::fstat(m_dirfd, &st);
    if ((st.st_mode & S_IFMT) != S_IFDIR)
        throw koios::exception(::std::string(toolpex::lazy_string_concater{} 
                + "It's not a directory!, directory path = " 
                + dir_path().string()));
}

bool dir_mutex::create_lock_file() const
{
    const int ret = ::openat(m_dirfd, lock_file_name().data(), 
                             O_CREAT | O_EXCL | O_CLOEXEC, 
                             S_IRWXU | S_IRGRP | S_IROTH);
    if (ret < 0)
    { 
        if (errno == EEXIST) return false;
        else if (errno == 0) return true;
        throw koios::exception(ret);
    }

    // auto close it.
    toolpex::unique_posix_fd{ ret };

    return true;
}

lazy_task<> dir_mutex::polling_lock_file(
    ::std::stop_token tk, 
    ::std::chrono::milliseconds period)
{
    co_await this_task::sleep_for(period);
    while (!tk.stop_requested())
    {
        if (this->create_lock_file())       
        {
            may_wake_next();
            co_return;
        }
        co_await this_task::sleep_for(period);
    }
}

void dir_mutex::cancel_all_polling() noexcept
{
    m_stop_src.request_stop();
    ::std::unique_lock lk{ m_pollers_lock };
    auto pollers = ::std::move(m_pollers);
    lk.unlock();

    for (auto& item : pollers)
    {
        item.get();
    }
}

dir_mutex::~dir_mutex() noexcept
{
    this->cancel_all_polling();
    this->unlock();
}
    
bool dir_mutex::hold_this_immediately()
{
    const bool success = this->create_lock_file();

    if (!success) 
    {
        ::std::lock_guard lk{ m_pollers_lock };
        m_pollers.emplace_back(
            this->polling_lock_file(m_stop_src.get_token(), polling_period())
                .run_and_get_future());
    }

    return success;
}

lazy_task<> dir_mutex::delete_lock_file()
{
    co_await uring::unlinkat(m_dirfd, lock_file_name());
}

void dir_mutex::unlock() noexcept
{
    if (!this->may_wake_next()) 
    {
        this->delete_lock_file().result();
    }
}

} // namespace koios
