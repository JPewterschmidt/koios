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

#include "toolpex/tic_toc.h"
#include "toolpex/unique_posix_fd.h"
#include "koios/tcp_server.h"
#include "koios/iouring_awaitables.h"
#include "koios/coroutine_mutex.h"
#include <string_view>
#include "koios/iouring_connect_aw.h"

#include "koios/latch.h"

using namespace koios;
using namespace ::std::chrono_literals;

tcp_server* sp{};

task<void> tcp_server_app(toolpex::unique_posix_fd client)
{
    static size_t count{};
    auto [addr, port] = toolpex::ip_address::getpeername(client);
    ::std::cout << "client: [addr: " << addr->to_string() << ", port: " << port << "]";

    ::std::string msg = "fuck you!!!!";

    ::std::array<char, 128> buffer{};

    auto recv_ret = co_await uring::recv(client, buffer);
    ::std::cout << recv_ret.nbytes_delivered() << "bytes delivered!" << ::std::endl;

    co_await uring::send(client, msg);
    ::std::string_view sv{ buffer.data(), recv_ret.nbytes_delivered() };
    if (sv.contains("stop") && sp && ++count > 10)
        sp->stop();

    co_return;
}

task<void> client_app()
{
    using namespace toolpex::ip_address_literals;
    using namespace ::std::string_view_literals;

    auto sock = co_await uring::connect_get_sock("::1"_ip, 8889);
    auto ret = co_await uring::send(sock, "fuck you"sv);
    while (ret.error_code())
    {
        co_await uring::send(sock, "fuck you"sv);
    }
}

task<void> server_emitter()
{
    using namespace toolpex::ip_address_literals;

    tcp_server server("::1"_ip, 8889);   
    sp = &server;
    co_await server.start(tcp_server_app);
    co_await client_app();
    co_await server.until_stop_async();   
    co_return;
}

int main()
try
{
    koios::runtime_init(3);

    server_emitter().result();

    koios::runtime_exit();
    
    return 0;
}
catch (const ::std::exception& e)
{
    ::std::cout << e.what() << ::std::endl;
    koios::runtime_exit();
}
