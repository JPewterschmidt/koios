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
    constinit size_t test_size{ 100000 };
    constinit size_t pool_size{ 10 };
}

int main()
{
    thread_pool tp{ pool_size };
    ::std::atomic_size_t count{ test_size };

    for (size_t i = 0; i < test_size / 10; ++i)
    {
        tp.enqueue([&]{ --count; });
        tp.enqueue([&]{ --count; });
        tp.enqueue([&]{ --count; });
        tp.enqueue([&]{ --count; });
        tp.enqueue([&]{ --count; });
        tp.enqueue([&]{ --count; });
        tp.enqueue([&]{ --count; });
        tp.enqueue([&]{ --count; });
        tp.enqueue([&]{ --count; });
        tp.enqueue([&]{ --count; });
    }
    
    tp.stop();
    ::std::cout << count << ::std::endl;
}
