#include <vector>
#include <functional>

#include "fmt/core.h"
#include "fmt/ranges.h"
#include "glog/logging.h"

#include "koios/task.h"
#include "koios/expected_task.h"
#include "koios/thread_pool.h"
#include "koios/from_result.h"
#include "koios/generator.h"

#include "toolpex/unique_resource.h"

#include <chrono>
#include <thread>
#include <iostream>
#include <future>
#include <ranges>
#include <iterator>

using namespace koios;

namespace 
{
    int result{};
    int count{};
    int flag{};
    int referd_obj{};

    ::std::binary_semaphore sem{0}; 

    task<int> for_with_scheduler()
    {
        if (++count >= 10) co_return 1;
        co_return co_await for_with_scheduler() + 1;
    }

    task<void> starter()
    {
        result = co_await for_with_scheduler();
        sem.release();
    }

    async_task<void> func1() { co_return; }
    sync_task<void> func2() { co_return; }
}

task<void> for_basic_test()
{
    flag = 1;
    co_return;
}

nodiscard_task<int> for_basic_test2()
{
    unsigned char buffer[1024];
    co_return 2;
}

task<int&> for_basic_test3()
{
    co_return referd_obj;
}

monad_task<int> for_et()
{
    co_return 1;
}

int main(int argc, char** argv)
{
    auto result = for_et()
        .transform([](int i) -> monad_task<int>{
                co_return ::std::error_code{ 1,::std::system_category() }; 
        }).transform([](int i) -> monad_task<int>{ 
            co_return i * 100; 
        }).run_and_get_future().get().get_value();
    ::std::cout << result;
}
