#ifndef KOIOS_IOURING_SENDMSG_AW_H
#define KOIOS_IOURING_SENDMSG_AW_H

#include "koios/macros.h"
#include "koios/iouring_aw.h"
#include "koios/iouring_detials.h"

#include "toolpex/unique_posix_fd.h"

namespace koios::uring
{
    class sendmsg : public detials::iouring_aw_for_data_deliver
    {
    public:
        sendmsg(const toolpex::unique_posix_fd& fd, 
                const ::msghdr* msg, 
                int flags = 0);
    };
}

#endif
