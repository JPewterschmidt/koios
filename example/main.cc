#include "toolpex/assert.h"
#include "koios/thread_pool.h"
#include "koios/runtime.h"
#include "koios/task_scheduler_concept.h"
#include "koios/task.h"
#include <chrono>
#include <iostream>
#include <fcntl.h>
#include <atomic>
#include <vector>
#include <fstream>

#include "koios/work_stealing_queue.h"
#include "koios/moodycamel_queue_wrapper.h"
#include "koios/this_task.h"
#include "koios/expected.h"
#include "koios/functional.h"
#include "koios/from_result.h"
#include "koios/generator.h"

#include "toolpex/tic_toc.h"
#include "toolpex/errret_thrower.h"
#include "toolpex/unique_posix_fd.h"
#include "koios/tcp_server.h"
#include "koios/iouring_awaitables.h"
#include "koios/coroutine_mutex.h"
#include <string_view>

#include "koios/unique_file_state.h"
#include "koios/task_release_once.h"
#include "koios/iouring_op_batch.h"

#include <fcntl.h>
#include <sys/stat.h>

using namespace koios;
using namespace ::std::chrono_literals;
using namespace ::std::string_view_literals;
using namespace toolpex::ip_address_literals;

namespace
{
    lazy_task<int> func()
    {
        co_return 1;
    }
    
    generator<int> gen()
    {
        for (int i{}; i < 10; ++i)
        {
            co_yield co_await func() + i; 
        }
        co_return;
    }

    lazy_task<> main_body()
    {
        co_await uring::unlink("./no-exists");

        co_return;
    }
}

int main()
try
{
    koios::runtime_init(1);
    main_body().result();
    koios::runtime_exit();
    ::std::cout << ::std::endl;
    return 0;
}
catch (const ::std::exception& e)
{
    ::std::cout << e.what() << ::std::endl;
    koios::runtime_exit();
}
