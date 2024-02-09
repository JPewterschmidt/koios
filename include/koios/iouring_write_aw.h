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

#ifndef KOIOS_IOURING_WRITE_AW_H
#define KOIOS_IOURING_WRITE_AW_H

#include <system_error>
#include <cerrno>
#include <string_view>

#include "koios/macros.h"
#include "koios/iouring_aw.h"
#include "koios/iouring_detials.h"

#include "toolpex/unique_posix_fd.h"

KOIOS_NAMESPACE_BEG

namespace uring
{
    class write : public detials::iouring_aw_for_data_deliver
    {
    public:
        write(const toolpex::unique_posix_fd& fd, 
              ::std::span<const unsigned char> buffer, 
              uint64_t offset = 0);

        write(const toolpex::unique_posix_fd& fd, 
              ::std::span<const ::std::byte> buffer, 
              uint64_t offset = 0);

        write(const toolpex::unique_posix_fd& fd, 
              ::std::span<const char> buffer, 
              uint64_t offset = 0);

        write(const toolpex::unique_posix_fd& fd, 
              ::std::string_view buffer,
              uint64_t offset = 0);

        write(::std::chrono::milliseconds timeout, 
              const toolpex::unique_posix_fd& fd, 
              ::std::span<const unsigned char> buffer, 
              uint64_t offset = 0);

        write(::std::chrono::milliseconds timeout, 
              const toolpex::unique_posix_fd& fd, 
              ::std::span<const ::std::byte> buffer, 
              uint64_t offset = 0);

        write(::std::chrono::milliseconds timeout,
              const toolpex::unique_posix_fd& fd, 
              ::std::span<const char> buffer, 
              uint64_t offset = 0);

        write(::std::chrono::milliseconds timeout, 
              const toolpex::unique_posix_fd& fd, 
              ::std::string_view buffer,
              uint64_t offset = 0);

        write(::std::chrono::system_clock::time_point timeout, 
              const toolpex::unique_posix_fd& fd, 
              ::std::span<const unsigned char> buffer, 
              uint64_t offset = 0);

        write(::std::chrono::system_clock::time_point timeout, 
              const toolpex::unique_posix_fd& fd, 
              ::std::span<const ::std::byte> buffer, 
              uint64_t offset = 0);

        write(::std::chrono::system_clock::time_point timeout,
              const toolpex::unique_posix_fd& fd, 
              ::std::span<const char> buffer, 
              uint64_t offset = 0);

        write(::std::chrono::system_clock::time_point timeout, 
              const toolpex::unique_posix_fd& fd, 
              ::std::string_view buffer,
              uint64_t offset = 0);
    };

    // `ioret_for_data_deliver` is return type of await_resume
}

KOIOS_NAMESPACE_END

#endif
