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

generator<int> gen_func()
{
    for (int i{}; i < 10; ++i)
    {
        co_yield i;
    }
}

task<void> func()
{
    for (auto i : gen_func())
        ;
    co_return;
}

int main()
try 
{
    //runtime_init(1, manually_stop);
    runtime_init(1);

    func().result();

    return 0;
} 
catch (const ::std::exception& e) 
{
    koios::log_error(e.what());
    return 1;
}
