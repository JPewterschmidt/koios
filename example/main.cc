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

#include "koios/iouring_read_aw.h"

task<void> test_read()
{
    ::std::string_view name{ "testfile1.txt" };
    toolpex::unique_posix_fd fd{ ::open(name.data(), 0) };

    ::std::array<unsigned char, 1024> bf{};
    auto ret = co_await uring::read(fd, bf);

    co_return;
}

task<void> emitter()
{
    ::std::vector<koios::future<void>> fvec;

    for (size_t i{}; i < 100; ++i)
    {
        fvec.push_back(test_read().run_and_get_future());
        fvec.push_back(test_read().run_and_get_future());
        fvec.push_back(test_read().run_and_get_future());
        fvec.push_back(test_read().run_and_get_future());
        fvec.push_back(test_read().run_and_get_future());
        fvec.push_back(test_read().run_and_get_future());
        fvec.push_back(test_read().run_and_get_future());
        fvec.push_back(test_read().run_and_get_future());
        fvec.push_back(test_read().run_and_get_future());
        fvec.push_back(test_read().run_and_get_future());
    }

    for (auto& f : fvec)
    {
        f.get();
    }

    co_return;
}

int main()
{
    koios::runtime_init(10);

    emitter().result();

    koios::runtime_exit();
    
    return 0;
}
