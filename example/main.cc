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

task<int> coro()
{
    ::std::this_thread::sleep_for(10min);
    co_return 1;
}

task<void> starter()
{
    result = co_await coro();
    ::std::cout << "coro: " << result << ::std::endl;
}

constinit size_t test_size{ 10000 };
constinit size_t pool_size{ 10 };

int main()
{
    thread_pool tp{ pool_size };
    ::std::atomic_size_t count{ test_size };

    for (size_t i = 0; i < test_size / 10; ++i)
    {
        tp.enqueue([&]{ --count; });
        tp.enqueue([&]{ --count; });
        tp.enqueue([&]{ --count; });
        tp.enqueue([&]{ --count; });
        tp.enqueue([&]{ --count; });
        tp.enqueue([&]{ --count; });
        tp.enqueue([&]{ --count; });
        tp.enqueue([&]{ --count; });
        tp.enqueue([&]{ --count; });
        tp.enqueue([&]{ --count; });
    }
    
    tp.stop();
    ::std::cout << count << ::std::endl;
    task_scheduler_concept auto& scheduler = koios::get_task_scheduler();
    scheduler.stop();
}
