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

task<> func()
{
    co_return;
}

emitter_task<> func2()
{
    co_return;
}

emitter_task<int> emitter(int i = 1)
{

    co_return i;
}

namespace
{
    bool hascopyed{ false };
    class lifetime
    {
    public:
        lifetime() = default;
        lifetime(const lifetime&) { hascopyed = true; }
        lifetime& operator= (const lifetime&) { hascopyed = true; return *this; }
        lifetime(lifetime&&) noexcept { ::std::cout << "ok" << ::std::endl; }
        lifetime& operator=(lifetime&&) noexcept { ::std::cout << "ok" << ::std::endl; return *this; }
    };
    
    task<lifetime> should_not_copy()
    {
        lifetime ret;
        co_return ret;
    }
}

int main()
try
{
    runtime_init(4);

    (void)should_not_copy().result();
    ::std::cout << hascopyed << ::std::endl;

    runtime_exit();
    
    return 0;
}
catch (const ::std::exception& e)
{
    ::std::cout << e.what() << ::std::endl;
    koios::runtime_exit();
}
