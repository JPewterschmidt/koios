#include "koios/thread_pool.h"
#include "koios/runtime.h"
#include "koios/task_scheduler_concept.h"
#include "koios/task.h"
#include <chrono>
#include <iostream>
#include <cassert>

#include "koios/work_stealing_queue.h"
#include "koios/moodycamel_queue_wrapper.h"
#include "koios/this_task.h"

#include "toolpex/tic_toc.h"

using namespace koios;
using namespace ::std::chrono_literals;

task<size_t> func2(size_t count = 0)
{
    if (count > 100)
        co_return 1;
    co_return 1 + co_await func2(count + 1);
}

task<size_t> func()
{
    size_t result{};

    ::std::vector<koios::future<size_t>> fvec{};

    for (size_t i{}; i < 10000; ++i)
    {
        fvec.emplace_back(func2().run_and_get_future());
        fvec.emplace_back(func2().run_and_get_future());
        fvec.emplace_back(func2().run_and_get_future());
        fvec.emplace_back(func2().run_and_get_future());
        fvec.emplace_back(func2().run_and_get_future());
        fvec.emplace_back(func2().run_and_get_future());
        fvec.emplace_back(func2().run_and_get_future());
        fvec.emplace_back(func2().run_and_get_future());
        fvec.emplace_back(func2().run_and_get_future());
        fvec.emplace_back(func2().run_and_get_future());
    }
    
    for (auto& i : fvec)
    { 
        result += i.get();
    }

    co_return result;
}

int main()
{
    koios::runtime_init(20);
    auto t = toolpex::tic();
    (void)func().result();
    ::std::cout << toolpex::toc(t) << ::std::endl;
    koios::runtime_exit();
    
    return 0;
}
