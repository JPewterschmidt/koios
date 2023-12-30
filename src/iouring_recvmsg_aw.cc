#include "koios/iouring_recvmsg_aw.h"
#include <system_error>
#include <liburing.h>
#include <cerrno>

using namespace koios;
using namespace uring;

static ::io_uring_sqe
init_helper(const toolpex::unique_posix_fd& fd, 
            ::msghdr* msg, 
            int flags)
{
    ::io_uring_sqe result{};
    ::io_uring_prep_recvmsg(&result, fd, msg, flags);
    return result;
}


recvmsg::recvmsg(const toolpex::unique_posix_fd& fd, 
                 ::msghdr* msg, 
                 int flags)
    : detials::iouring_aw_for_data_deliver(init_helper(fd, msg, flags))
{
    errno = 0;
}
