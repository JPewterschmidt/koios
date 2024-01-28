#include "koios/iouring_cancel_aw.h"

namespace koios::uring
{
    static ::io_uring_sqe
    init_helper(void* handle, int flags = 0)
    {
        ::io_uring_sqe result{};
        ::io_uring_prep_cancel(&result, handle, flags); 
        return result;
    }

    cancel_first::cancel_first(void* handle)
        : iouring_aw{init_helper(handle)}
    {
    }

    cancel_all::cancel_all(void* handle)
        : iouring_aw{init_helper(handle, IORING_ASYNC_CANCEL_ALL)}
    {
    }

    cancel_first::cancel_first(::std::coroutine_handle<> handle)
        : iouring_aw{init_helper(handle.address())}
    {
    }

    cancel_all::cancel_all(::std::coroutine_handle<> handle)
        : iouring_aw{init_helper(handle.address(), IORING_ASYNC_CANCEL_ALL)}
    {
    }
}
