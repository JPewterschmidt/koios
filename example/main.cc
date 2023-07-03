#include <vector>

#include "fmt/core.h"
#include "fmt/ranges.h"

#include "koios/task.h"
#include "koios/thread_pool.h"

#include <chrono>
#include <thread>
#include <iostream>

using namespace koios;
using namespace ::std::chrono_literals;

int result{};

task<int&> coro2()
{
    co_return result;
}

task<int> coro()
{
    int& val = co_await coro2();
    val = 100;
    ::std::cout << "coro2: " << result << ::std::endl;
    co_return 1;
}

task<void> starter()
{
    result = co_await coro();
}

constinit size_t test_size{ 10000 };
constinit size_t pool_size{ 10 };

int main()
{
    task_scheduler_concept auto& scheduler = koios::get_task_scheduler();

    scheduler.enqueue(starter());
    scheduler.stop();
}
