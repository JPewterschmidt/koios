#include "koios/iouring_send_aw.h"
#include <system_error>
#include <liburing.h>
#include <cerrno>

namespace koios::uring { 

static ::io_uring_sqe
init_helper(const toolpex::unique_posix_fd& fd, 
            const void* buffer, size_t size_nbytes, 
            int flags)
{
    ::io_uring_sqe result{};
    ::io_uring_prep_send(
        &result, fd, 
        buffer, size_nbytes,
        flags
    );
    return result;
}

send::send(const toolpex::unique_posix_fd& fd, 
           ::std::span<const unsigned char> buffer, 
           int flags)
    : detials::iouring_aw_for_data_deliver(init_helper(
           fd, buffer.data(), buffer.size_bytes(), flags))
{
    errno = 0;
}

send::send(const toolpex::unique_posix_fd& fd, 
           ::std::string_view buffer,
           int flags)
    : detials::iouring_aw_for_data_deliver(init_helper(
           fd, buffer.data(), buffer.size(), flags))
{
    errno = 0;
}

} // namespace koios::uring
