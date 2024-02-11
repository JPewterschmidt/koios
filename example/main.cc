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
    task<> mute_client_app()
    {
        auto sock = co_await uring::connect_get_sock("::1"_ip, 8890);
        ::std::array<::std::byte, 4> buffer{};

        co_await this_task::sleep_for(1s);
        co_await uring::send(sock, "fuck");
        
        co_return;
    }

    task<> recv_timeout_server(toolpex::unique_posix_fd client) 
    {
        ::std::array<::std::byte, 4> buffer{};
        ::std::error_code ec{};
        size_t recved = co_await uring::recv_fill_buffer(3s, client, buffer, 0, ec);
        ::std::cout << ec.message() << ::std::endl;
        co_return;
    }

    emitter_task<> emit_recv_timeout_test()
    {
        tcp_server s{ "::1"_ip, 8890 };
        co_await s.start(recv_timeout_server);
        co_await mute_client_app();
        s.stop();
        co_await s.until_stop_async();
        co_return;
    }
}

int main()
try
{
    koios::runtime_init(4);
    emit_recv_timeout_test().result();
    koios::runtime_exit();
    return 0;
}
catch (const ::std::exception& e)
{
    ::std::cout << e.what() << ::std::endl;
    koios::runtime_exit();
}
