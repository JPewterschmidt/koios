#include <iostream>
#include "koios/task.h"
#include "toolpex/tic_toc.h"

#include <chrono>

using namespace koios;
using namespace toolpex;
using namespace ::std::chrono_literals;

#include <semaphore>
::std::binary_semaphore bs{0};

task<int> func()
{
    co_return 1;
}

task<void> starter()
{
    auto b = tic();
    for (size_t i = 0; i < 10000; ++i)
        co_await func();   
    ::std::cout << toc(b) << ::std::endl;
}

int main()
try 
{
    koios::runtime_init(1);

    auto f = starter().run_and_get_future();
    f.get();

    koios::runtime_exit();
    return 0;
} 
catch (...) 
{
    ::std::cout << "catched" << ::std::endl;
    return 1;
}
