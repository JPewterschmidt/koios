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

    cancel_all::cancel_all(::std::coroutine_handle<> handle)
        : cancel_all{ handle.address() }
    {
    }
}
