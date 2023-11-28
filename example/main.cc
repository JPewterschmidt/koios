#include <iostream>
#include "koios/task.h"
#include "koios/event_loop.h"
#include "toolpex/tic_toc.h"
#include "koios/timer.h"
#include "koios/this_task.h"

#include <chrono>

using namespace koios;
using namespace toolpex;
using namespace ::std::chrono_literals;

::std::binary_semaphore bs{0};

task<void> func()
{
    ::std::cout << "before sleep" << ::std::endl;
    co_await this_task::sleep_await(500ms);
    ::std::cout << "after sleep" << ::std::endl;
}

int main()
try 
{
    //runtime_init(1, manually_stop);
    runtime_init(1);

    func().result();

    return 0;
} 
catch (...) 
{
    ::std::cout << "catched" << ::std::endl;
    return 1;
}
