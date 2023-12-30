#include "koios/iouring_read_aw.h"
#include <system_error>
#include <liburing.h>

using namespace koios;
using namespace uring;

static ::io_uring_sqe 
init_helper(const toolpex::unique_posix_fd& fd, 
            ::std::span<unsigned char> buffer, 
            uint64_t offset = 0)
{
    ::io_uring_sqe result{};
    ::io_uring_prep_read(
        &result, fd, 
        buffer.data(), 
        static_cast<unsigned int>(buffer.size_bytes()), 
        offset
    );
    return result;
}

read::read(const toolpex::unique_posix_fd& fd, 
           ::std::span<unsigned char> buffer, 
           uint64_t offset)
    : detials::iouring_aw_for_data_deliver(init_helper(fd, buffer, offset))
{
    errno = 0;
}
