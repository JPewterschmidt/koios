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

#include <cerrno>
#include "koios/iouring_write_aw.h"

namespace koios::uring { 

static 
::io_uring_sqe
init_helper(const toolpex::unique_posix_fd& fd, 
            const void* buffer, size_t size_nbytes, 
            uint64_t offset)
{
    ::io_uring_sqe result{};
    ::io_uring_prep_write(
        &result, fd, 
        buffer, static_cast<unsigned>(size_nbytes), 
        offset
    );
    return result;
}

write::write(const toolpex::unique_posix_fd& fd, 
             ::std::span<const ::std::byte> buffer, 
             uint64_t offset)
    : detials::iouring_aw_for_data_deliver(init_helper(fd, buffer.data(), buffer.size_bytes(), offset))
{
    errno = 0;
}

write::write(const toolpex::unique_posix_fd& fd, 
             ::std::string_view buffer, 
             uint64_t offset)
    : detials::iouring_aw_for_data_deliver(init_helper(fd, buffer.data(), buffer.size(), offset))
{
    errno = 0;
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

} // namespace koios::uring
