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
#include "koios/task_release_once.h"

#include <fcntl.h>
#include <sys/stat.h>

using namespace koios;
using namespace ::std::chrono_literals;
using namespace ::std::string_view_literals;
using namespace toolpex::ip_address_literals;

namespace
{
    ::std::unique_ptr<tcp_server> sp{};
    task<bool> mute_client_app()
    {
        auto sock = co_await uring::connect_get_sock("::1"_ip, 8890);
        ::std::array<::std::byte, 4> buffer{};

        ::std::error_code ec;
        co_await uring::recv_fill_buffer(50min, sock, buffer, 0, ec);

        if (buffer[0] == ::std::byte{}) co_return false;
        co_return true;
    }

    task<bool> recv_timeout_server(toolpex::unique_posix_fd client) 
    {
        ::std::array<::std::byte, 128> buffer{};
        size_t recved = co_await uring::recv_fill_buffer(3s, client, buffer);
        if (recved == 0) 
        {
            co_await uring::send(client, "fuck you");
            co_return true;
        }
        co_return false;
    }

    emitter_task<bool> emit_recv_timeout_test()
    {
        sp.reset(new tcp_server("::1"_ip, 8890));
        co_await sp->start(recv_timeout_server);

        if (!co_await mute_client_app())
        {
            sp->stop();
            co_await sp->until_stop_async();
            sp = nullptr;
            ::std::cout << "shit1" << ::std::endl;
            co_return false;
        }
        sp->stop();
        co_await sp->until_stop_async();
        sp = nullptr;
        co_return true;
    }
}

int main()
try
{
    koios::runtime_init(4);
    ::std::cout << emit_recv_timeout_test().result() << ::std::endl;
    koios::runtime_exit();
    return 0;
}
catch (const ::std::exception& e)
{
    ::std::cout << e.what() << ::std::endl;
    koios::runtime_exit();
}
