#include "koios/iouring_socket_aw.h"

namespace koios::uring { 

static 
::io_uring_sqe
init_helper(int domain, int type, int protocal, unsigned int flags)
{
    ::io_uring_sqe result{};
    ::io_uring_prep_socket(&result, domain, type, protocal, flags);
    return result;
}

uring::socket::socket(int domain, int type, int protocal, unsigned int flags)
    : detials::iouring_aw_for_socket(init_helper(domain, type, protocal, flags))
{
    errno = 0;
}

} // namespace koios::uring
