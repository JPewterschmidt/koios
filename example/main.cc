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

int main()
try 
{
    runtime_init(1);

    auto t = tic();
    delayed_scheduler ds(3s);
    receiver().result_on(ds);
    ::std::cout << toc(t);

    return 0;
} 
catch (const ::std::exception& e) 
{
    koios::log_error(e.what());
    return 1;
}
