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
        +[] -> task<int> { co_return 1; }, 
        +[] -> task<int> { co_return 1; }, 
        +[] -> task<int> { co_return 1; }, 
        +[] -> task<int> { co_return 1; }, 
    };

    int result{};

    for (const auto& i : tvec)
    {
        result += co_await i();
    }

    co_return result;
}

task<void> starter()
{
    co_await coro();
}

int main()
try {
    auto t = tic();
    koios::runtime_init(12);

    ::std::this_thread::sleep_for(10s);

    koios::runtime_exit();
    return 0;

} catch (...) {
    return 1;
}
