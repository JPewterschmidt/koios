#include "koios/iouring_cancel_aw.h"

namespace koios::uring
{
    cancel_first::cancel_first(void* handle)
    {
        ::io_uring_prep_cancel(sqe_ptr(), handle, 0); 
    }

    cancel_all::cancel_all(void* handle)
    {
        ::io_uring_prep_cancel(sqe_ptr(), handle, IORING_ASYNC_CANCEL_ALL); 
    }

    cancel_first::cancel_first(::std::coroutine_handle<> handle)
        : cancel_first{ handle.address() }
    {
    }

    cancel_first::cancel_first(uint64_t handle)
    {
        ::io_uring_prep_cancel64(sqe_ptr(), handle, 0);
    }

    cancel_all::cancel_all(::std::coroutine_handle<> handle)
        : cancel_all{ handle.address() }
    {
    }

    cancel_all::cancel_all(uint64_t handle)
    {
        ::io_uring_prep_cancel64(sqe_ptr(), handle, IORING_ASYNC_CANCEL_ALL);
    }

    cancel_any::cancel_any(const toolpex::unique_posix_fd& fd, ::std::coroutine_handle<> handle)
        : cancel_any(fd, handle.address())
    {
    }

    cancel_any::cancel_any(const toolpex::unique_posix_fd& fd, void* handle)
        : cancel_any(int(fd), handle)
    {
    }

    cancel_any::cancel_any(const toolpex::unique_posix_fd& fd, uint64_t handle)
        : cancel_any(int(fd), handle)
    {
    }

    cancel_any::cancel_any(int fd, ::std::coroutine_handle<> handle)
        : cancel_any(fd, handle.address())
    {
    }

    cancel_any::cancel_any(int fd, void* handle)
    {
        ::io_uring_prep_cancel(sqe_ptr(), handle, IORING_ASYNC_CANCEL_ANY); 
        sqe_ptr()->fd = fd;
    }

    cancel_any::cancel_any(int fd, uint64_t handle)
    {
        ::io_uring_prep_cancel64(sqe_ptr(), handle, IORING_ASYNC_CANCEL_ANY); 
        sqe_ptr()->fd = fd;
    }
}
