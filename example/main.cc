#include <vector>

#include "fmt/core.h"
#include "fmt/ranges.h"

#include "koios/task.h"
#include "koios/thread_pool.h"
#include "koios/from_result.h"

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

    task<int> for_with_scheduler()
    {
        if (++count >= 10) co_return 1;
        ::std::cout << "count: " << count << ::std::endl;
        co_return co_await for_with_scheduler() + 1;
    }

    task<void> starter()
    {
        int i = co_await from_result(10);
        ::std::cout << i << ::std::endl;
        co_await for_with_scheduler();
        sem.release();
    }
}

int main()
{
    starter().run();
}
