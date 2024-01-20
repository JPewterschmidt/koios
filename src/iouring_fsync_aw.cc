#include "koios/iouring_fsync_aw.h"

namespace koios::uring
{
    static ::io_uring_sqe
    init_helper(const toolpex::unique_posix_fd& fd, int flags)
    {
        ::io_uring_sqe result{};
        ::io_uring_prep_fsync(&result, fd, flags);

        return result;
    }

    fsync::fsync(const toolpex::unique_posix_fd& fd)
        : iouring_aw(init_helper(fd, 0))
    {
    }

    fdatasync::fdatasync(const toolpex::unique_posix_fd& fd)
        : iouring_aw(init_helper(fd, IORING_FSYNC_DATASYNC))
    {
    }
}
