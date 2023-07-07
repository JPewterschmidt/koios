#include <vector>

#include "fmt/core.h"
#include "fmt/ranges.h"

#include "koios/task.h"
#include "koios/thread_pool.h"
#include "koios/from_result.h"
#include "koios/generator.h"

#include <chrono>
#include <thread>
#include <iostream>

using namespace koios;
using namespace ::std::chrono_literals;

namespace 
{
    int result{};
    int count{};
    ::std::binary_semaphore sem{0}; 

    ::std::vector<int> ivec{ 1,2,3,4,5 };

    generator<int&> g1()
    {
        co_await ::std::suspend_always{};
        for (auto i : ivec)
            co_yield i;
    }
}

int main()
{
    auto g = g1();
    
    g.move_next();
    auto v = g.current_value();
    fmt::print("{}\n", v);

    g.move_next();
    v = g.current_value();
    fmt::print("{}\n", v);

    g.move_next();
    v = g.current_value();
    fmt::print("{}\n", v);
}
