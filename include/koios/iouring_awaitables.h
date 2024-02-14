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

#ifndef KOIOS_IOURING_AWAITABLES_H
#define KOIOS_IOURING_AWAITABLES_H

#include "koios/iouring_aw.h"
#include "koios/exceptions.h"
#include "koios/iouring_op_functions.h"

#include "koios/task.h"
#include "toolpex/ipaddress.h"
#include <cstddef>

namespace koios::uring
{
    task<toolpex::unique_posix_fd> 
    connect_get_sock(toolpex::ip_address::ptr addr, 
                     ::in_port_t port, 
                     unsigned int socket_flags = 0);

    task<toolpex::unique_posix_fd> 
    bind_get_sock(toolpex::ip_address::ptr addr, in_port_t port, 
                  bool reuse_port = true, bool reuse_addr = true,
                  unsigned int flags = 0);

    task<>
    append_all(const toolpex::unique_posix_fd& fd, 
              ::std::span<const ::std::byte> buffer);

    template<typename CharT>
    task<>
    inline append_all(const toolpex::unique_posix_fd& fd, 
                      ::std::basic_string_view<CharT, ::std::char_traits<CharT>> buffer)
    {
        return append_all(fd, ::std::as_bytes(
            ::std::span{ buffer.begin(), buffer.end() }
        ));
    }

    task<>
    append_all(const toolpex::unique_posix_fd& fd, 
              ::std::span<const ::std::byte> buffer, 
              ::std::error_code& ec_out) noexcept;

    template<typename CharT>
    inline task<>
    append_all(const toolpex::unique_posix_fd& fd, 
               ::std::basic_string_view<CharT, ::std::char_traits<CharT>> buffer,
               ::std::error_code& ec_out) noexcept
    {
        return append_all(fd, ::std::as_bytes(
            ::std::span{ buffer.begin(), buffer.end() }
        ), ec_out);
    }

    task<size_t>
    recv_fill_buffer(const toolpex::unique_posix_fd& fd, 
                     ::std::span<::std::byte> buffer, 
                     int flags,
                     ::std::error_code& ec, 
                     ::std::chrono::system_clock::time_point timeout 
                         = ::std::chrono::system_clock::time_point::max()) noexcept;

    task<size_t>
    recv_fill_buffer(const toolpex::unique_posix_fd& fd, 
                     ::std::span<::std::byte> buffer, 
                     int flags, 
                     ::std::error_code& ec, 
                     ::std::chrono::milliseconds dura) noexcept
    {
        return recv_fill_buffer(fd, buffer, flags, ec,
                                dura + ::std::chrono::system_clock::now()
                               );
    }

    task<size_t>
    recv_fill_buffer(const toolpex::unique_posix_fd& fd, 
                     ::std::span<::std::byte> buffer, 
                     int flags = 0, 
                     ::std::chrono::system_clock::time_point timeout 
                         = ::std::chrono::system_clock::time_point::max())
    {
        ::std::error_code ec{};
        auto result = co_await recv_fill_buffer(fd, buffer, flags, ec, timeout);
        if (ec && ec != std_canceled_ec()) throw uring_exception(ec);
        co_return result;
    }

    task<size_t>
    recv_fill_buffer(const toolpex::unique_posix_fd& fd, 
                     ::std::span<::std::byte> buffer, 
                     int flags, 
                     ::std::chrono::milliseconds dura)
    {
        ::std::error_code ec{};
        auto result = co_await recv_fill_buffer(fd, buffer, flags, ec, dura);
        if (ec && ec != std_canceled_ec()) throw uring_exception(ec);
        co_return result;
    }

    task<size_t>
    read_fill_buffer(const toolpex::unique_posix_fd& fd, 
                     ::std::span<::std::byte> buffer, 
                     int offset,
                     ::std::error_code& ec, 
                     ::std::chrono::system_clock::time_point timeout 
                         = ::std::chrono::system_clock::time_point::max()) noexcept;

    task<size_t>
    read_fill_buffer(const toolpex::unique_posix_fd& fd, 
                     ::std::span<::std::byte> buffer, 
                     int offset, 
                     ::std::error_code& ec, 
                     ::std::chrono::milliseconds dura) noexcept
    {
        return read_fill_buffer(fd, buffer, offset, ec,
                                dura + ::std::chrono::system_clock::now()
                               );
    }

    task<size_t>
    read_fill_buffer(const toolpex::unique_posix_fd& fd, 
                     ::std::span<::std::byte> buffer, 
                     int offset = 0, 
                     ::std::chrono::system_clock::time_point timeout 
                         = ::std::chrono::system_clock::time_point::max())
    {
        ::std::error_code ec{};
        auto result = co_await read_fill_buffer(fd, buffer, offset, ec, timeout);
        if (ec && ec != std_canceled_ec()) throw uring_exception(ec);
        co_return result;
    }

}

#endif
