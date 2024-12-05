#ifndef KOIOS_TCP_UTIL_H
#define KOIOS_TCP_UTIL_H

#include <system_error>

#include "koios/task.h"

#include "toolpex/unique_posix_fd.h"

namespace koios
{

/*! \brief Make sure the data in the sending buffer has at least arrived to the peer OS.
 *  \retval true All the data got ACK.
 *  \retval false timeout or other error occured. you can use the ec version to take a look what's error occured.
 *
 *  \see https://blog.netherlabs.nl/articles/2009/01/18/the-ultimate-so_linger-page-or-why-is-my-tcp-not-reliable
 */
task<bool> deplete_send_buffer(const toolpex::unique_posix_fd& fd, ::std::chrono::milliseconds timeout = {});
task<bool> deplete_send_buffer(const toolpex::unique_posix_fd& fd, ::std::error_code& ec, ::std::chrono::milliseconds timeout = {});

} // namespace koios

#endif
