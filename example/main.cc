#include <iostream>
#include "koios/task.h"
#include "toolpex/tic_toc.h"

#include <chrono>

using namespace koios;
using namespace toolpex;
using namespace ::std::chrono_literals;

task<int> coro()
{
    ::std::vector tvec{ 
        +[] -> task<int> { co_return 1; }, 
        +[] -> task<int> { co_return 2; }, 
        +[] -> task<int> { co_return 3; }, 
        +[] -> task<int> { co_return 4; }, 
        +[] -> task<int> { co_return 5; }, 
    };

    int result{};

    for (const auto& i : tvec)
    {
        int val = co_await i();
        ::std::cout << val << ::std::endl;
        result += val;
    }

    co_return result;
}

task<void> starter() noexcept
try 
{
    co_await coro();
} 
catch (const koios::exception& e)
{
    e.log();
}
catch (const std::exception& e)
{
    ::std::cout << e.what();
}
catch (...)
{
    ::std::cout << "starter have caught comething, exiting" << ::std::endl;
}

int main()
try 
{
    auto t = tic();
    koios::runtime_init(12);

    starter().run();
    koios::runtime_exit();

    auto now = ::std::chrono::high_resolution_clock::now();
    ::std::cout << ::std::chrono::duration_cast<::std::chrono::nanoseconds>(now - t).count() << ::std::endl;

    return 0;
} 
catch (...) 
{
    ::std::cout << "catched" << ::std::endl;
    return 1;
}
