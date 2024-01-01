#ifndef KOIOS_IOURING_ACCEPT_AW_H
#define KOIOS_IOURING_ACCEPT_AW_H

#include <system_error>
#include <cerrno>
#include <span>

#include "koios/macros.h"
#include "koios/iouring_aw.h"
#include "koios/iouring_detials.h"

#include "toolpex/unique_posix_fd.h"
#include "toolpex/ipaddress.h"

#include <sys/socket.h>

namespace koios::uring
{
    class accept : public detials::iouring_aw_for_accept
    {
    public:
        accept(const toolpex::unique_posix_fd& fd, int flags = 0)
            : detials::iouring_aw_for_accept{ fd, flags }
        {
        }
    };
}

#endif
