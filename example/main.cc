#include "koios/thread_pool.h"
#include "koios/runtime.h"
#include "koios/task_scheduler_concept.h"
#include "koios/task.h"
#include <chrono>
#include <iostream>
#include <cassert>
#include <fcntl.h>
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

using namespace koios;
using namespace ::std::chrono_literals;

task<void> tcp_server_app(toolpex::unique_posix_fd client)
{
    ::std::string msg = "fuck you!!!!";
    co_await uring::send(client, msg);

    co_return;
}

task<void> emitter()
{
    using namespace toolpex::ip_address_literals;

    tcp_server server("127.0.0.1"_ip, 8889);   
    co_await server.start(tcp_server_app);
    
    co_await koios::this_task::sleep_for(1h);
    server.stop();

    co_return;
}

int main()
try
{
    koios::runtime_init(3);

    auto p = toolpex::tic();
    auto f = emitter().run_and_get_future();
    f.get();

    ::std::cout << toolpex::toc(p) << ::std::endl;

    koios::runtime_exit();
    
    return 0;
}
catch (const ::std::exception& e)
{
    ::std::cout << e.what() << ::std::endl;
    koios::runtime_exit();
}
