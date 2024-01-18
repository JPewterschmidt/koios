#ifndef KOIOS_UNIQUE_FILE_STATE_H
#define KOIOS_UNIQUE_FILE_STATE_H

#include "koios/macros.h"
#include "koios/task.h"
#include "koios/coroutine_mutex.h"
#include "toolpex/unique_posix_fd.h"

#include <cstddef>
#include <filesystem>
#include <span>

KOIOS_NAMESPACE_BEG

class unique_file_state
{
public:
    unique_file_state(::std::filesystem::path path);

    ::std::string name() const noexcept { return m_path; }
    task<::std::error_code> append_async(::std::span<const ::std::byte> buffer) noexcept;
    void close() { m_fd.close(); }

private:
    ::std::filesystem::path m_path;
    toolpex::unique_posix_fd m_fd;
    koios::mutex m_fs_io_lock;
};

KOIOS_NAMESPACE_END

#endif
