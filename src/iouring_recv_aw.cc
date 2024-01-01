#include "koios/iouring_recv_aw.h"
#include <system_error>
#include <liburing.h>
#include <cerrno>

namespace koios::uring { 

static ::io_uring_sqe
init_helper(const toolpex::unique_posix_fd& fd, 
            ::std::span<unsigned char> buffer, 
            int flags)
{
    ::io_uring_sqe result{};
    ::io_uring_prep_recv(
        &result, fd, 
        buffer.data(), static_cast<size_t>(buffer.size_bytes()), 
        flags
    );
    return result;
}

recv::recv(const toolpex::unique_posix_fd& fd, 
           ::std::span<unsigned char> buffer, 
           int flags)
    : detials::iouring_aw_for_data_deliver(init_helper(fd, buffer, flags))
{
    errno = 0;
}

} // namespace koios::uring
