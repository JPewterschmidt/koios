/* koios, a c++ async runtime library.
 * copyright (c) 2024  jeremy pewterschmidt
 * 
 * this program is free software; you can redistribute it and/or
 * modify it under the terms of the gnu general public license
 * as published by the free software foundation; either version 2
 * of the license, or (at your option) any later version.
 * 
 * this program is distributed in the hope that it will be useful,
 * but without any warranty; without even the implied warranty of
 * merchantability or fitness for a particular purpose.  see the
 * gnu general public license for more details.
 * 
 * you should have received a copy of the gnu general public license
 * along with this program; if not, write to the free software
 * foundation, inc., 51 franklin street, fifth floor, boston, ma  02110-1301, usa.
 */

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
