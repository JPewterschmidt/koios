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

#include "koios/iouring_op_functions.h"

namespace koios::uring
{

read_write_aw 
read(const toolpex::unique_posix_fd& fd, 
     ::std::span<::std::byte> buffer, 
     uint64_t offset, 
     ::std::chrono::system_clock::time_point tmot)
{
    read_write_aw result;
    result.batch().prep_read(fd, buffer, offset).timeout(tmot);
    return result;
}

read_write_aw 
read(const toolpex::unique_posix_fd& fd, 
     ::std::span<::std::byte> buffer, 
     uint64_t offset,
     ::std::chrono::milliseconds tmot)
{
    read_write_aw result;
    result.batch().prep_read(fd, buffer, offset).timeout(tmot);
    return result;
}

read_write_aw 
read(const toolpex::unique_posix_fd& fd, 
     ::std::span<char> buffer, 
     uint64_t offset,
     ::std::chrono::system_clock::time_point tmot)
{
    return read(fd, ::std::as_writable_bytes(buffer), offset, tmot);
}

read_write_aw 
read(const toolpex::unique_posix_fd& fd, 
     ::std::span<char> buffer, 
     uint64_t offset, 
     ::std::chrono::milliseconds tmot)
{
    return read(fd, ::std::as_writable_bytes(buffer), offset, tmot + ::std::chrono::system_clock::now());
}

read_write_aw 
write(const toolpex::unique_posix_fd& fd, 
      ::std::span<const ::std::byte> buffer, 
      uint64_t offset, 
      ::std::chrono::system_clock::time_point tmot)
{
    read_write_aw result{};
    result.batch().prep_write(fd, buffer, offset).timeout(tmot);
    return result;
}

read_write_aw 
write(const toolpex::unique_posix_fd& fd, 
      ::std::span<const ::std::byte> buffer, 
      uint64_t offset, 
      ::std::chrono::milliseconds tmot)
{
    read_write_aw result{};
    result.batch().prep_write(fd, buffer, offset).timeout(tmot);
    return result;
}

read_write_aw 
write(const toolpex::unique_posix_fd& fd, 
      ::std::span<const char> buffer, 
      uint64_t offset, 
      ::std::chrono::system_clock::time_point tmot)
{
    return write(fd, ::std::as_bytes(buffer), offset, tmot);
}

read_write_aw 
write(const toolpex::unique_posix_fd& fd, 
      ::std::span<const char> buffer, 
      uint64_t offset, 
      ::std::chrono::milliseconds tmot)
{
    return write(fd, ::std::as_bytes(buffer), offset, tmot + ::std::chrono::system_clock::now());
}

recv_send_aw
send(const toolpex::unique_posix_fd& fd, 
     ::std::span<const ::std::byte> buffer, 
     int flags, 
     ::std::chrono::system_clock::time_point tmot)
{
    recv_send_aw result{};
    result.batch().prep_send(fd, buffer, flags).timeout(tmot);
    return result;
}

recv_send_aw
send(const toolpex::unique_posix_fd& fd, 
     ::std::span<const char> buffer, 
     int flags,
     ::std::chrono::system_clock::time_point tmot)
{
    return send(fd, ::std::as_bytes(buffer), flags, tmot);
}

recv_send_aw
send(const toolpex::unique_posix_fd& fd, 
     ::std::span<const ::std::byte> buffer, 
     int flags, 
     ::std::chrono::milliseconds tmot)
{
    recv_send_aw result{};
    result.batch().prep_send(fd, buffer, flags).timeout(tmot);
    return result;
}

recv_send_aw
send(const toolpex::unique_posix_fd& fd, 
     ::std::span<const char> buffer, 
     int flags, 
     ::std::chrono::milliseconds tmot)
{
    return send(fd, ::std::as_bytes(buffer), flags, tmot + ::std::chrono::system_clock::now());
}

recv_send_aw
recv(const toolpex::unique_posix_fd& fd, 
     ::std::span<::std::byte> buffer, 
     int flags, 
     ::std::chrono::system_clock::time_point tmot)
{
    recv_send_aw result{};
    result.batch().prep_recv(fd, buffer, flags).timeout(tmot);
    return result;
}

recv_send_aw
recv(const toolpex::unique_posix_fd& fd, 
     ::std::span<char> buffer, 
     int flags, 
     ::std::chrono::system_clock::time_point tmot)
{
    return recv(fd, ::std::as_writable_bytes(buffer), flags, tmot);
}

recv_send_aw
recv(const toolpex::unique_posix_fd& fd, 
     ::std::span<::std::byte> buffer, 
     int flags, 
     ::std::chrono::milliseconds tmot)
{
    recv_send_aw result{};
    result.batch().prep_recv(fd, buffer, flags).timeout(tmot);
    return result;
}

recv_send_aw
recv(const toolpex::unique_posix_fd& fd, 
     ::std::span<char> buffer, 
     int flags, 
     ::std::chrono::milliseconds tmot)
{
    return recv(fd, ::std::as_writable_bytes(buffer), flags, tmot + ::std::chrono::system_clock::now());
}

normal_aw 
rename(const ::std::filesystem::path& from, 
       const ::std::filesystem::path& to)
{
    normal_aw result;
    result.batch().prep_rename(from, to);
    return result;
}

normal_aw
renameat(const toolpex::unique_posix_fd& olddir, const ::std::filesystem::path& from, 
         const toolpex::unique_posix_fd& newdir, const ::std::filesystem::path& to, 
         int flags)
{
    normal_aw result;
    result.batch().prep_renameat(olddir, from, newdir, to, flags);
    return result;
}

normal_aw
renameat_noreplace(const toolpex::unique_posix_fd& olddir, 
                   const ::std::filesystem::path& from, 
                   const toolpex::unique_posix_fd& newdir, 
                   const ::std::filesystem::path& to)
{
    normal_aw result;
    result.batch().prep_renameat_noreplace(olddir, from, newdir, to);
    return result;
}

socket_aw 
socket(int domain, int type, int protocal, unsigned int flags)
{
    socket_aw result{};
    result.batch().prep_socket(domain, type, protocal, flags);
    return result;
}

posix_result_aw 
openat(const toolpex::unique_posix_fd& fd, 
       ::std::filesystem::path path, 
       int flags, mode_t mode)
{
    posix_result_aw result{};
    result.batch().prep_openat(fd, ::std::move(path), flags, mode);
    return result;
}

normal_aw 
unlink(::std::filesystem::path path, int flags)
{
    normal_aw result{};
    result.batch().prep_unlink(::std::move(path), flags);
    return result;
}

normal_aw 
unlinkat(const toolpex::unique_posix_fd& fd, 
         ::std::filesystem::path path, int flags)
{
    normal_aw result{};
    result.batch().prep_unlinkat(fd, ::std::move(path), flags);
    return result;
}

normal_aw fsync(const toolpex::unique_posix_fd& fd)
{
    normal_aw result{};
    result.batch().prep_fsync(fd);
    return result;
}

normal_aw fdatasync(const toolpex::unique_posix_fd& fd)
{
    normal_aw result{};
    result.batch().prep_fdatasync(fd);
    return result;
}

normal_aw 
sync_file_range(const toolpex::unique_posix_fd& fd, 
                unsigned len, uint64_t offset, int flags)
{
    normal_aw result{};
    result.batch().prep_sync_file_range(fd, len, offset, flags);
    return result;
}

recv_send_aw
sendmsg(const toolpex::unique_posix_fd& fd, 
        const ::msghdr* msg, 
        int flags, 
        ::std::chrono::system_clock::time_point tmot)
{
    recv_send_aw result{};
    result.batch().prep_sendmsg(fd, msg, flags).timeout(tmot);
    return result;
}

recv_send_aw
recvmsg(const toolpex::unique_posix_fd& fd, 
        ::msghdr* msg, 
        int flags, 
        ::std::chrono::system_clock::time_point tmot)
{
    recv_send_aw result{};
    result.batch().prep_recvmsg(fd, msg, flags).timeout(tmot);
    return result;
}

connect_aw
connect(const toolpex::unique_posix_fd& fd, 
        toolpex::ip_address::ptr addr, 
        ::in_port_t port, 
        ::std::chrono::system_clock::time_point tmot)
{
    connect_aw result{};
    result.batch().prep_connect(fd, ::std::move(addr), port).timeout(tmot);
    return result;
}

accept_aw
accept(const toolpex::unique_posix_fd& fd, 
       ::std::chrono::system_clock::time_point tmot) noexcept
{
    accept_aw result{};
    result.batch().prep_accept(fd).timeout(tmot);
    return result;
}

cancel_aw cancel_any(const toolpex::unique_posix_fd& fd, uintptr_t userdata)
{
    cancel_aw result{};
    if (userdata) result.batch().prep_cancel_any(fd, userdata);
    else          result.batch().prep_cancel_any(fd);
    return result;
}

cancel_aw cancel_first(uintptr_t userdata)
{
    cancel_aw result{};
    result.batch().prep_cancel_first(userdata);
    return result;
}

cancel_aw cancel_all(uintptr_t userdata)
{
    cancel_aw result{};
    result.batch().prep_cancel_first(userdata);
    return result;
}

nop_aw nop() 
{
    nop_aw result{};
    result.batch().prep_nop();
    return result;
}

} // namespace koios::uring
