#include <vector>

#include "fmt/core.h"
#include "fmt/ranges.h"

#include "koios/task.h"
#include "koios/thread_pool.h"
#include "koios/from_result.h"
#include "koios/generator.h"

#include "toolpex/unique_resource.h"

#include <chrono>
#include <thread>
#include <iostream>
#include <future>
#include <ranges>
#include <iterator>

using namespace koios;

koios::generator<int> g(int last_val)
{
    for (int i = 1; i <= last_val; ++i)
        co_yield i;
}

void func(::std::ranges::range auto& r)
{
    for (const auto& i : r)
    {
        ::std::cout << i << ::std::endl;
    }
}


koios::task<int> task1(int count = 0)
{
    if (count == 10) 
    {
        co_return 1;
    }
    co_return co_await task1(count + 1);
}

namespace 
{
    int result{};
    int count{};
    ::std::binary_semaphore sem{0}; 

    task<int> for_with_scheduler()
    {
        if (++count >= 10) co_return 1;
        co_return co_await for_with_scheduler() + 1;
    }

    task<void> starter()
    {
        result = co_await for_with_scheduler();
        sem.release();
    }

    async_task<void> func1() { co_return; }
    sync_task<void> func2() { co_return; }
}

int main()
{
    for (size_t i{}; i < 100000; ++i)
        task1().run();

    get_task_scheduler().stop();

    return 0;
}
