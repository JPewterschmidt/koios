#include <iostream>
#include "koios/task.h"
#include "koios/event_loop.h"
#include "toolpex/tic_toc.h"
#include "koios/timer.h"
#include "koios/this_task.h"
#include "koios/generator.h"

#include <chrono>

using namespace koios;
using namespace toolpex;
using namespace ::std::chrono_literals;

::std::binary_semaphore bs{0};

task<void> func1()
{
    ::std::cout << ("1") << ::std::endl;
    co_return;
}

task<void> func2()
{
    ::std::cout << ("2") << ::std::endl;
    co_return;
}

task<void> func3()
{
    ::std::cout << ("3") << ::std::endl;
    bs.release();
    co_return;
}



int main()
try 
{
    runtime_init(11);

    get_task_scheduler().add_event<timer_event_loop>(1s, func1());
    get_task_scheduler().add_event<timer_event_loop>(2s, func2());
    get_task_scheduler().add_event<timer_event_loop>(3s, func3());

    bs.acquire();
    
    runtime_exit();

    return 0;
} 
catch (const ::std::exception& e) 
{
    koios::log_error(e.what());
    return 1;
}
