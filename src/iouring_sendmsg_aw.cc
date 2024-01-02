#include "koios/iouring_sendmsg_aw.h"

#include <system_error>
#include <liburing.h>
#include <cerrno>

namespace koios::uring { 

static ::io_uring_sqe
init_helper(const toolpex::unique_posix_fd& fd, 
            ::msghdr* hdr,
            int flags)
{
    ::io_uring_sqe result{};
    ::io_uring_prep_sendmsg(&result, fd, hdr, flags);
    return result;
}

sendmsg::sendmsg(const toolpex::unique_posix_fd& fd, 
                 ::msghdr* msg, 
                 int flags)
    : detials::iouring_aw_for_data_deliver(init_helper(fd, msg, flags))
{
    errno = 0;
}

} // namespace koios::uring
