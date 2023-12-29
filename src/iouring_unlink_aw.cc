#include "koios/iouring_unlink_aw.h"
#include <liburing.h>

using namespace koios::io;

static 
::io_uring_sqe 
init_helper(::std::string_view path, int flags)
{
    ::io_uring_sqe result{};
    ::io_uring_prep_unlink(&result, path.data(), flags);

    return result;
}

unlink::unlink(::std::string_view path, int flags)
    : iouring_aw{ init_helper(path, flags) }
{
}

unlink::unlink(::std::filesystem::path path, int flags)
    : iouring_aw{ init_helper(path.string(), flags) }
{
}
