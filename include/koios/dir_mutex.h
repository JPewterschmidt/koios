#ifndef KOIOS_DIR_MUTEX_H
#define KOIOS_DIR_MUTEX_H

#include <atomic>
#include <stop_token>
#include <filesystem>
#include "koios/async_awaiting_hub.h"
#include "koios/task.h"

namespace koios
{

class dir_mutex;

class dir_mutex_guard
{
public:
    dir_mutex_guard(dir_mutex* parent) noexcept
        : m_parent{ parent }
    {
    }

    dir_mutex_guard(dir_mutex_guard&& other) noexcept;
    dir_mutex_guard& operator=(dir_mutex_guard&& other) noexcept;

    ~dir_mutex_guard() noexcept { unlock(); }
    void unlock() noexcept;

private:
    dir_mutex* m_parent{};
};

class dir_mutex_acq_aw
{
public:
    dir_mutex_acq_aw(dir_mutex* parent) noexcept
        : m_parent{ parent }
    {
    }

    bool await_ready() const;
    void await_suspend(task_on_the_fly t) noexcept;
    dir_mutex_guard await_resume() noexcept;

private:
    dir_mutex* m_parent{};
};

class dir_mutex : private async_awaiting_hub
{
public:
    dir_mutex(::std::filesystem::path p);
    
    const ::std::filesystem::path& path() const noexcept { return m_path; }
    dir_mutex_acq_aw acquire() noexcept { return { this }; }
    void cancel_all_polling() noexcept;
    void unlock() noexcept;
    ~dir_mutex() noexcept;
    
private:
    friend class dir_mutex_acq_aw;
    bool hold_this_immediately();
    static constexpr ::std::string_view lock_file_name() { return "koios_dir_lock"; }
    eager_task<> polling_lock_file(::std::stop_token tk);
    bool create_lock_file() const;
    eager_task<> delete_lock_file();

private:
    ::std::filesystem::path m_path;
    toolpex::unique_posix_fd m_dirfd;
    ::std::stop_source m_stop_src;
    ::std::vector<koios::future<void>> m_pollers;
}; 

} // namespace koios

#endif
