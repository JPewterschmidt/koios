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

using namespace koios;
using namespace ::std::chrono_literals;
using namespace toolpex::ip_address_literals;

task<void> tcp_app(uring::accepted_client client)
{
    ::std::cout << client << ::std::endl;   

    co_return;
}

task<void> func()
{
    static koios::mutex m;
    {
        auto lk = co_await m.acquire();
        //::std::cout << "func ok 1" << ::std::endl;
        lk.unlock();
    
        co_await lk.lock();
        //::std::cout << "func ok 2" << ::std::endl;
    }
    
    co_return;
}

task<void> emitter()
{
    ::std::vector<koios::future<void>> fvec{};

    for (size_t i{}; i < 10000; ++i)
    {
        fvec.emplace_back(func().run_and_get_future());
        fvec.emplace_back(func().run_and_get_future());
        fvec.emplace_back(func().run_and_get_future());
        fvec.emplace_back(func().run_and_get_future());
        fvec.emplace_back(func().run_and_get_future());
        fvec.emplace_back(func().run_and_get_future());
        fvec.emplace_back(func().run_and_get_future());
    }

    for (auto& f : fvec)
        f.get();

    co_return;
}

int main()
try
{
    koios::runtime_init(2);

    auto p = toolpex::tic();
    emitter().result();
    ::std::cout << toolpex::toc(p) << ::std::endl;

    koios::runtime_exit();
    
    return 0;
}
catch (const ::std::exception& e)
{
    ::std::cout << e.what() << ::std::endl;
    koios::runtime_exit();
}
