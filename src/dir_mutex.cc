#include <chrono>
#include <fstream>
#include <sys/stat.h>
#include <fcntl.h>
#include "toolpex/exceptions.h"
#include "toolpex/errret_thrower.h"
#include "toolpex/functional.h"
#include "koios/dir_mutex.h"
#include "koios/iouring_awaitables.h"
#include "koios/this_task.h"

namespace koios
{

namespace fs = ::std::filesystem;

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
    : m_path{ ::std::move(p) }, 
      m_dirfd{ et << ::open(m_path.c_str(), O_DIRECTORY) }
{
    ::stat st{};
    ::fstat(m_dirfd, &st);
    if (st.st_mode & S_IFMT != S_IFDIR)
        throw koios::exception(toolpex::lazy_string_concater{} 
                + "It's not a directory!, path = " 
                + m_path.string());
}

bool dir_mutex::create_lock_file() const
{
    const int ret = ::openat(m_dirfd, lock_file_name().data(), 
                             O_CREAT | O_EXCL | O_CLOEXEC);
    if (ret == EEXIST) return false;
    else if (ret == 0) return true;

    throw koios::exception(ret);

    return {};
}

task<> dir_mutex::polling_lock_file(::std::stop_token tk)
{
    using namespace ::std::chrono_literals;
    co_await this_task::sleep_for(100ms);
    while (!tk.stop_requested())
    {
        if (create_lock_file())       
        {
            may_wake_next();
            co_return;
        }
        co_await this_task::sleep_for(100ms);
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
    const bool success = create_lock_file();

    if (!success) 
    {
        m_pollers.emplace_back(
            polling_lock_file(m_stop_src.get_token())
                .run_and_get_future());
    }

    return success;
}

static task<> delete_lock_file(const auto& path)
{
    // TODO
}

void dir_mutex::unlock() noexcept
{
    if (!this->may_wake_next()) 
    {
        delete_lock_file(path()).result();
    }
}

} // namespace koios
