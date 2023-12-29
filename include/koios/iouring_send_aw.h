#ifndef KOIOS_IOURING_SEND_AW_H
#define KOIOS_IOURING_SEND_AW_H

#include "koios/macros.h"
#include "koios/iouring_aw.h"
#include "koios/iouring_detials.h"

#include "toolpex/unique_posix_fd.h"

namespace koios::io
{
    class send : public iouring_aw
    {
    public:
        send(const toolpex::unique_posix_fd& fd, 
             ::std::span<const unsigned char> buffer, 
             int flags = 0);
        ioret_for_data_deliver await_resume();
    };
}

#endif
