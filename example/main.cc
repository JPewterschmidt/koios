#include <vector>

#include "fmt/core.h"
#include "fmt/ranges.h"

#include "koios/task.h"
#include "koios/thread_pool.h"

#include <chrono>
#include <thread>
#include <iostream>

using namespace koios;
using namespace ::std::chrono_literals;


namespace 
{
    int result{};
    ::std::binary_semaphore sem{0}; 

    task<int> for_with_scheduler()
    {
        static size_t count{};
        if (++count >= 10)
            co_return 1;
        int result = co_await for_with_scheduler() + 1;
        ::std::cout << result << ::std::endl;
        co_return result;
    }

    task<void> starter()
    {
        result = co_await for_with_scheduler();
        ::std::cout << "starter: " << result << ::std::endl;
        sem.release();
    }
}

int main()
{
    auto t = starter();
    t.run_sync();
    ::std::cout << result;
}
