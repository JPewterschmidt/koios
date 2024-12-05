#include <sys/ioctl.h>
#ifdef __linux__
#include <linux/sockios.h>
#endif

#include "koios/tcp_util.h"
#include "koios/this_task.h"

using namespace ::std::chrono_literals;

namespace koios
{

task<bool> 
deplete_send_buffer(const toolpex::unique_posix_fd& fd, 
                    ::std::error_code& ec,
                    ::std::chrono::milliseconds timeout)
{
    auto now = ::std::chrono::system_clock::now();
    const auto timeout_tp = now + timeout;
    constexpr auto wait_dura = 1ms;
    int outstanding = -1, ret{};
    for (;;)
    {
        // Assume that the fd has been enabled O_NONBLOCK
        if ((ret = ::ioctl(fd, SIOCOUTQ, &outstanding)) == -1)
        {
            ec = { errno, ::std::system_category() };
            co_return false;
        }
        else if (!outstanding)
        {
            co_return true;
        }
        now = ::std::chrono::system_clock::now();
        if (now + wait_dura < timeout_tp)
            co_await this_task::sleep_for(wait_dura);       
        else break;
    }
    co_return false;
}

task<bool> 
deplete_send_buffer(const toolpex::unique_posix_fd& fd, 
                    ::std::chrono::milliseconds timeout)
{
    [[maybe_unused]] ::std::error_code ec;
    co_return co_await deplete_send_buffer(fd, ec, timeout);
}

} // namespace koios
