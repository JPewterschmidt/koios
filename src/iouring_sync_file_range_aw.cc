#include "koios/iouring_sync_file_range_aw.h"

namespace koios::uring
{
    static ::io_uring_sqe
    init_helper(const toolpex::unique_posix_fd& fd, 
                unsigned len, uint64_t offset, int flags)
    {
        ::io_uring_sqe result{};
        ::io_uring_prep_sync_file_range(&result, fd, len, offset, flags);

        return result;
    }

    sync_file_range::
    sync_file_range(const toolpex::unique_posix_fd& fd, 
                    unsigned len, uint64_t offset, int flags)
        : iouring_aw(init_helper(fd, len, offset, flags))
    {
    }
}
