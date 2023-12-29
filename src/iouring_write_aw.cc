#include <cerrno>
#include "koios/iouring_write_aw.h"

using namespace koios::io;

static 
::io_uring_sqe
init_helper(const toolpex::unique_posix_fd& fd, 
            ::std::span<const unsigned char> buffer, 
            uint64_t offset)
{
    ::io_uring_sqe result{};
    ::io_uring_prep_write(
        &result, fd, 
        buffer.data(), 
        static_cast<unsigned int>(buffer.size_bytes()), 
        offset
    );
    return result;
}

write::write(const toolpex::unique_posix_fd& fd, 
             ::std::span<const unsigned char> buffer, 
             uint64_t offset)
    : detials::iouring_aw_for_data_deliver(init_helper(fd, buffer, offset))
{
    errno = 0;
}
