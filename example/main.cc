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

using namespace koios;
using namespace ::std::chrono_literals;

task<void> tcp_app(uring::accepted_client client)
{
    ::std::cout << client << ::std::endl;   

    co_return;
}

task<void> emitter()
{
    using namespace toolpex;
    tcp_server server{ "127.0.0.1"_ip, 8890, tcp_app };
    co_await this_task::sleep_for(1min);
    co_return;
}

int main()
{
    koios::runtime_init(10);

    emitter().result();

    koios::runtime_exit();
    
    return 0;
}
