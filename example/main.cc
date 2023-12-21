#include "koios/thread_pool.h"
#include "koios/runtime.h"
#include "koios/task_scheduler_concept.h"
#include "koios/task.h"
#include <chrono>
#include <iostream>

using namespace koios;
using namespace ::std::chrono_literals;


using namespace koios;

namespace 
{
    int flag{};
    int referd_obj{};

    task<void> for_basic_test()
    {
        flag = 1;
        co_return;
    }

    task<int> for_basic_test2()
    {
        co_return 2;
    }

    task<int&> for_basic_test3()
    {
        co_return referd_obj;
    }

    sync_task<int> for_sync_task()
    {
        co_return 1;
    }
    
    nodiscard_task<int> for_nodiscard()
    {
        co_return 1;
    }

    task<::std::chrono::high_resolution_clock::time_point> func()
    {
        co_return ::std::chrono::high_resolution_clock::now();
    }
}

constinit size_t test_size{ 10000 };
constinit size_t pool_size{ 10 };

task<bool> test_main()
{
    co_return true;
}

int main()
{
    koios::runtime_init(10);
    auto x = test_main().result();

    return 0;
}
