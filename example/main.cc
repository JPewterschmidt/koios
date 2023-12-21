#include "koios/thread_pool.h"
#include "koios/runtime.h"
#include "koios/delayed_scheduler.h"
#include "koios/task_scheduler_concept.h"
#include "koios/delayed_scheduler.h"
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

int main()
{
    koios::runtime_init(10);

    delayed_scheduler ds{ 50ms };
    const auto now = ::std::chrono::high_resolution_clock::now();
    const auto ret_tp = func().result_on(ds);

    koios::runtime_exit();

    return 0;
}
