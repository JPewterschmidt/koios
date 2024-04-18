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

#ifndef KOIOS_IOURING_OP_FUNCTIONS_H
#define KOIOS_IOURING_OP_FUNCTIONS_H

#include "koios/iouring_single_op_aw.h"
#include "toolpex/unique_posix_fd.h"
#include "toolpex/concepts_and_traits.h"
#include <concepts>
#include <ranges>
#include <span>

namespace koios::uring
{

read_write_aw 
read(const toolpex::unique_posix_fd& fd, 
     ::std::span<::std::byte> buffer, 
     uint64_t offset = 0, 
     ::std::chrono::system_clock::time_point tmot 
         = ::std::chrono::system_clock::time_point::max());

read_write_aw 
read(const toolpex::unique_posix_fd& fd, 
     ::std::span<::std::byte> buffer, 
     uint64_t offset,
     ::std::chrono::milliseconds tmot);

read_write_aw 
read(const toolpex::unique_posix_fd& fd, 
     ::std::span<char> buffer, 
     uint64_t offset = 0, 
     ::std::chrono::system_clock::time_point tmot 
        = ::std::chrono::system_clock::time_point::max());

read_write_aw 
read(const toolpex::unique_posix_fd& fd, 
     ::std::span<char> buffer, 
     uint64_t offset, 
     ::std::chrono::milliseconds tmot);

read_write_aw 
write(const toolpex::unique_posix_fd& fd, 
      ::std::span<const ::std::byte> buffer, 
      uint64_t offset = 0, 
      ::std::chrono::system_clock::time_point tmot 
          = ::std::chrono::system_clock::time_point::max());

read_write_aw 
write(const toolpex::unique_posix_fd& fd, 
      ::std::span<const ::std::byte> buffer, 
      uint64_t offset, 
      ::std::chrono::milliseconds tmot);

read_write_aw 
write(const toolpex::unique_posix_fd& fd, 
      ::std::span<const char> buffer, 
      uint64_t offset = 0, 
      ::std::chrono::system_clock::time_point tmot 
          = ::std::chrono::system_clock::time_point::max());

read_write_aw 
write(const toolpex::unique_posix_fd& fd, 
      ::std::span<const char> buffer, 
      uint64_t offset, 
      ::std::chrono::milliseconds tmot);

recv_send_aw
send(const toolpex::unique_posix_fd& fd, 
     ::std::span<const ::std::byte> buffer, 
     int flags = 0, 
     ::std::chrono::system_clock::time_point tmot 
        = ::std::chrono::system_clock::time_point::max());

recv_send_aw
send(const toolpex::unique_posix_fd& fd, 
     ::std::span<const char> buffer, 
     int flags = 0, 
     ::std::chrono::system_clock::time_point tmot 
        = ::std::chrono::system_clock::time_point::max());

recv_send_aw
send(const toolpex::unique_posix_fd& fd, 
     ::std::span<const ::std::byte> buffer, 
     int flags, 
     ::std::chrono::milliseconds tmot);

recv_send_aw
send(const toolpex::unique_posix_fd& fd, 
     ::std::span<const char> buffer, 
     int flags, 
     ::std::chrono::milliseconds tmot);

recv_send_aw
recv(const toolpex::unique_posix_fd& fd, 
     ::std::span<::std::byte> buffer, 
     int flags = 0, 
     ::std::chrono::system_clock::time_point tmot 
        = ::std::chrono::system_clock::time_point::max());

recv_send_aw
recv(const toolpex::unique_posix_fd& fd, 
     ::std::span<char> buffer, 
     int flags = 0, 
     ::std::chrono::system_clock::time_point tmot 
        = ::std::chrono::system_clock::time_point::max());

recv_send_aw
recv(const toolpex::unique_posix_fd& fd, 
     ::std::span<::std::byte> buffer, 
     int flags, 
     ::std::chrono::milliseconds tmot);

recv_send_aw
recv(const toolpex::unique_posix_fd& fd, 
     ::std::span<char> buffer, 
     int flags, 
     ::std::chrono::milliseconds tmot);

normal_aw 
rename(const ::std::filesystem::path& from, 
       const ::std::filesystem::path& to);

normal_aw
renameat(const toolpex::unique_posix_fd& olddir, const ::std::filesystem::path& from, 
         const toolpex::unique_posix_fd& newdir, const ::std::filesystem::path& to, 
         int flags = 0);

normal_aw
renameat_noreplace(const toolpex::unique_posix_fd& olddir, 
                   const ::std::filesystem::path& from, 
                   const toolpex::unique_posix_fd& newdir, 
                   const ::std::filesystem::path& to);

recv_send_aw
sendmsg(const toolpex::unique_posix_fd& fd, 
        const ::msghdr* msg, 
        int flags = 0, 
        ::std::chrono::system_clock::time_point tmot 
            = ::std::chrono::system_clock::time_point::max());

recv_send_aw
recvmsg(const toolpex::unique_posix_fd& fd, 
        ::msghdr* msg, 
        int flags = 0, 
        ::std::chrono::system_clock::time_point tmot 
            = ::std::chrono::system_clock::time_point::max());

socket_aw socket(int domain, int type, int protocal, unsigned int flags = 0);

connect_aw
connect(const toolpex::unique_posix_fd& fd, 
        toolpex::ip_address::ptr addr, 
        ::in_port_t port, 
        ::std::chrono::system_clock::time_point tmot 
            = ::std::chrono::system_clock::time_point::max());

inline connect_aw
connect(const toolpex::unique_posix_fd& fd, 
        toolpex::ip_address::ptr addr, 
        ::in_port_t port, 
        ::std::chrono::milliseconds tmot)
{
    return connect(fd, addr, port, tmot + ::std::chrono::system_clock::now());
}

normal_aw unlink(::std::filesystem::path path, int flags = 0);
normal_aw unlinkat(const toolpex::unique_posix_fd& fd, 
                   ::std::filesystem::path path, int flags = 0);
normal_aw fsync(const toolpex::unique_posix_fd& fd);
normal_aw fdatasync(const toolpex::unique_posix_fd& fd);

normal_aw 
sync_file_range(const toolpex::unique_posix_fd& fd, 
                unsigned len, uint64_t offset, int flags = 0);

accept_aw
accept(const toolpex::unique_posix_fd& fd, 
       ::std::chrono::system_clock::time_point tmot 
           = ::std::chrono::system_clock::time_point::max()) noexcept;

inline accept_aw
accept(const toolpex::unique_posix_fd& fd, 
       ::std::chrono::milliseconds tmot) noexcept
{
    return accept(fd, tmot + ::std::chrono::system_clock::now());
}

cancel_aw cancel_any(const toolpex::unique_posix_fd& fd, uintptr_t userdata = 0);

inline cancel_aw cancel_any(const toolpex::unique_posix_fd& fd, void* userdata = nullptr)
{
    return cancel_any(fd, reinterpret_cast<uintptr_t>(userdata));
}

cancel_aw cancel_first(uintptr_t userdata);
cancel_aw cancel_all(uintptr_t userdata);

nop_aw nop();

} // koios::uring

#endif
