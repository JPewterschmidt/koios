#ifndef KOIOS_IOURING_FSYNC_AW_H
#define KOIOS_IOURING_FSYNC_AW_H

#include "koios/iouring_aw.h"
#include "toolpex/unique_posix_fd.h"
#include <cstdint>

namespace koios::uring
{
    class fsync : public iouring_aw
    {
    public:
        fsync(const toolpex::unique_posix_fd& fd);
    };

    class fdatasync : public iouring_aw
    {
    public:
        fdatasync(const toolpex::unique_posix_fd& fd);
    };
}

#endif
