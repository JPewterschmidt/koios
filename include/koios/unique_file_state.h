/* Koios, A c++ async runtime library.
 * Copyright (C) 2024  Jeremy Pewterschmidt
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

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

    unique_file_state(unique_file_state&& other) noexcept
        : m_path{ ::std::move(other.m_path) }, 
          m_fd{ ::std::move(other.m_fd) }
    {
    }

    unique_file_state& operator=(unique_file_state&& other) noexcept
    {
        m_path  = ::std::move(other.m_path);
        m_fd    = ::std::move(other.m_fd);
        return *this;
    }

    ::std::string name() const { return m_path; }
    task<::std::error_code> append_async(::std::span<const ::std::byte> buffer) noexcept;
    void close() noexcept { m_fd.close(); }

private:
    ::std::filesystem::path m_path;
    toolpex::unique_posix_fd m_fd;
    mutable koios::mutex m_fs_io_lock;
};

KOIOS_NAMESPACE_END

#endif
