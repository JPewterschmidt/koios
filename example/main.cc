#include <iostream>
#include "koios/task.h"
#include "koios/event_loop.h"
#include "toolpex/tic_toc.h"

#include <chrono>

using namespace koios;
using namespace toolpex;
using namespace ::std::chrono_literals;

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
}


int main()
try 
{
    koios::runtime_init(11);

    for_basic_test().run_and_get_future().get();   

    koios::runtime_exit();
    return 0;
} 
catch (...) 
{
    ::std::cout << "catched" << ::std::endl;
    return 1;
}
