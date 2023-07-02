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

void func()
{
    ::std::cout << "ok" << ::std::endl;
}

task<int> coro2()
{
    ::std::cout << "ok2" << ::std::endl;
    co_return 2;
}

task<int> coro()
{
    co_await coro2();
    ::std::cout << "ok2" << ::std::endl;
    co_return 1;
}

int main()
{
    task_scheduler_concept auto& schr = get_task_scheduler();
    schr.enqueue(coro());
}
