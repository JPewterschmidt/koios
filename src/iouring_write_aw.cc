/* koios, a c++ async runtime library.
 * copyright (c) 2024  jeremy pewterschmidt
 * 
 * this program is free software; you can redistribute it and/or
 * modify it under the terms of the gnu general public license
 * as published by the free software foundation; either version 2
 * of the license, or (at your option) any later version.
 * 
 * this program is distributed in the hope that it will be useful,
 * but without any warranty; without even the implied warranty of
 * merchantability or fitness for a particular purpose.  see the
 * gnu general public license for more details.
 * 
 * you should have received a copy of the gnu general public license
 * along with this program; if not, write to the free software
 * foundation, inc., 51 franklin street, fifth floor, boston, ma  02110-1301, usa.
 */

#include "koios/iouring_write_aw.h"

namespace koios::uring { 

write::write(const toolpex::unique_posix_fd& fd, 
             ::std::span<const ::std::byte> buffer, 
             uint64_t offset)
{
    ::io_uring_prep_write(
        sqe_ptr(), fd, 
        buffer.data(), static_cast<unsigned>(buffer.size_bytes()), 
        offset
    );
}

write::write(const toolpex::unique_posix_fd& fd, 
             ::std::string_view buffer, 
             uint64_t offset)
{
    ::io_uring_prep_write(
        sqe_ptr(), fd, 
        buffer.data(), static_cast<unsigned>(buffer.size()), 
        offset
    );
}

write::write(const toolpex::unique_posix_fd& fd, 
             ::std::span<const unsigned char> buffer, 
             uint64_t offset)
    : write(fd, as_bytes(buffer), offset)
{
}

write::write(const toolpex::unique_posix_fd& fd, 
             ::std::span<const char> buffer, 
             uint64_t offset)
    : write(fd, as_bytes(buffer), offset)
{
}

write::write(::std::chrono::milliseconds timeout, 
             const toolpex::unique_posix_fd& fd, 
             ::std::span<const unsigned char> buffer, 
             uint64_t offset)
    : write(fd, buffer, offset)
{
    set_timeout(timeout);
}

write::write(::std::chrono::milliseconds timeout, 
             const toolpex::unique_posix_fd& fd, 
             ::std::span<const ::std::byte> buffer, 
             uint64_t offset)
    : write(fd, buffer, offset)
{
    set_timeout(timeout);
}

write::write(::std::chrono::milliseconds timeout,
             const toolpex::unique_posix_fd& fd, 
             ::std::span<const char> buffer, 
             uint64_t offset)
    : write(fd, buffer, offset)
{
    set_timeout(timeout);
}

write::write(::std::chrono::milliseconds timeout, 
             const toolpex::unique_posix_fd& fd, 
             ::std::string_view buffer,
             uint64_t offset)
    : write(fd, buffer, offset)
{
    set_timeout(timeout);
}

write::write(::std::chrono::system_clock::time_point timeout, 
             const toolpex::unique_posix_fd& fd, 
             ::std::span<const unsigned char> buffer, 
             uint64_t offset)
    : write(fd, buffer, offset)
{
    set_timeout(timeout);
}

write::write(::std::chrono::system_clock::time_point timeout, 
             const toolpex::unique_posix_fd& fd, 
             ::std::span<const ::std::byte> buffer, 
             uint64_t offset)
    : write(fd, buffer, offset)
{
    set_timeout(timeout);
}

write::write(::std::chrono::system_clock::time_point timeout,
             const toolpex::unique_posix_fd& fd, 
             ::std::span<const char> buffer, 
             uint64_t offset)
    : write(fd, buffer, offset)
{
    set_timeout(timeout);
}

write::write(::std::chrono::system_clock::time_point timeout, 
             const toolpex::unique_posix_fd& fd, 
             ::std::string_view buffer,
             uint64_t offset)
    : write(fd, buffer, offset)
{
    set_timeout(timeout);
}

} // namespace koios::uring
