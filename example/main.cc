#include <iostream>
#include "koios/task.h"
#include "koios/event_loop.h"
#include "toolpex/tic_toc.h"
#include "koios/timer.h"
#include "koios/this_task.h"
#include "koios/generator.h"
#include "koios/delayed_scheduler.h"

#include <chrono>
#include <algorithm>
#include <numeric>
#include <coroutine>

using namespace koios;
using namespace toolpex;
using namespace ::std::chrono_literals;

::std::binary_semaphore bs{0};

task<void> func()
{
    throw ::std::runtime_error{""};
    co_return;
}

task<void> receiver()
{
    try
    {
        co_await func();
    }
    catch (...)
    {
        ::std::cout << "ok" << ::std::endl;
    }

    co_return;
}

#include "koios/event_loop_concepts.h"

int main()
try 
{
    ::std::cout << koios::event_loop_concept<timer_event_loop> << ::std::endl;

    return 0;
} 
catch (const ::std::exception& e) 
{
    koios::log_error(e.what());
    return 1;
}
