#include "koios/thread_pool.h"
#include "koios/runtime.h"
#include "koios/task_scheduler_concept.h"
#include "koios/task.h"
#include <chrono>
#include <iostream>
#include <cassert>
#include <fcntl.h>
#include <atomic>
#include <vector>
#include <fstream>

#include "koios/work_stealing_queue.h"
#include "koios/moodycamel_queue_wrapper.h"
#include "koios/this_task.h"
#include "koios/expected.h"
#include "koios/functional.h"

#include "toolpex/tic_toc.h"
#include "toolpex/errret_thrower.h"
#include "toolpex/unique_posix_fd.h"
#include "koios/tcp_server.h"
#include "koios/iouring_awaitables.h"
#include "koios/coroutine_mutex.h"
#include <string_view>
#include "koios/iouring_connect_aw.h"

#include "koios/unique_file_state.h"

#include <fcntl.h>
#include <sys/stat.h>

using namespace koios;
using namespace ::std::chrono_literals;
using namespace ::std::string_view_literals;

namespace
{
    ::std::unique_ptr<tcp_server> sp{};
    ::std::atomic_bool flag{ false };

    emitter_task<void> tcp_server_app(toolpex::unique_posix_fd client) noexcept
    try
    {
        ::std::string msg = "fuck you!!!!";
        ::std::array<char, 128> buffer{};

        const auto recv_ret = co_await uring::recv(client, buffer);

        co_await uring::send(client, msg);
        ::std::string_view sv{ buffer.data(), recv_ret.nbytes_delivered() };
        if (sv.contains("stop"))
            flag.store(true), sp->stop();

        co_return;
    }
    catch (...)
    {
        co_return;
    }

    task<void> client_app() noexcept
    try
    {
        using namespace toolpex::ip_address_literals;
        using namespace ::std::string_view_literals;

        auto sock = co_await uring::connect_get_sock("::1"_ip, 8890);
        co_await uring::send(sock, "fuck you, and stop."sv);
        co_await uring::send(sock, "fuck you, and stop."sv);
        co_await uring::send(sock, "fuck you, and stop."sv);
        co_await uring::send(sock, "fuck you, and stop."sv);
        co_await uring::send(sock, "fuck you, and stop."sv);
        co_await uring::send(sock, "fuck you, and stop."sv);
        co_await uring::send(sock, "fuck you, and stop."sv);

        co_return;
    }
    catch (...)
    {
        co_return;
    }

    emitter_task<bool> emit_test()
    {
        auto now = ::std::chrono::system_clock::now();
        co_await this_task::sleep_until(now + 3s);
        co_return true;
    }
}

int main()
try
{
    runtime_init(4);
    ::std::cout << "runtime inited" << ::std::endl;

    ::std::cout << emit_test().result() << ::std::endl;

    runtime_exit();

    return 0;
}
catch (const ::std::exception& e)
{
    ::std::cout << e.what() << ::std::endl;
    koios::runtime_exit();
}
