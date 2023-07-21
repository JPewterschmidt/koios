#include <iostream>
#include "koios/task.h"

using namespace koios;

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
    koios::runtime_init(12);

    ::std::vector<::std::future<void>> fvec{};

    for (size_t i{}; i < 1000; ++i)
        fvec.emplace_back(starter().run_and_get_future());

    for (auto& i : fvec)
        i.get();

    return koios::runtime_exit();

} catch (...) {
    return 1;
}
