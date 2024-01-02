#ifndef KOIOS_IOURING_RECVMSG_AW_H
#define KOIOS_IOURING_RECVMSG_AW_H

#include <system_error>
#include <cerrno>
#include <span>
#include <sys/types.h>
#include <sys/socket.h>

#include "koios/macros.h"
#include "koios/iouring_aw.h"
#include "koios/iouring_detials.h"

#include "toolpex/unique_posix_fd.h"

namespace koios::uring
{
    class recvmsg : public detials::iouring_aw_for_data_deliver
    {
    public:
        recvmsg(const toolpex::unique_posix_fd& fd, 
                ::msghdr* msg, 
                int flags = 0);
    };
}

#endif
