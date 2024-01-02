#ifndef KOIOS_IOURING_SYNC_FILE_RANGE_H
#define KOIOS_IOURING_SYNC_FILE_RANGE_H

#include "koios/iouring_aw.h"
#include "toolpex/unique_posix_fd.h"

#include <cstdint>

namespace koios::uring
{
    class sync_file_range : public iouring_aw
    {
    public:
        sync_file_range(const toolpex::unique_posix_fd& fd, 
                        unsigned len, uint64_t offset, int flags = 0);
    };
}

#endif
