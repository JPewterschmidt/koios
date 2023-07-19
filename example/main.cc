#include <vector>
#include <functional>

#include "fmt/core.h"
#include "fmt/ranges.h"

#include "koios/task.h"
#include "koios/monad_task.h"
#include "koios/thread_pool.h"
#include "koios/from_result.h"
#include "koios/generator.h"
#include "koios/invocable_queue_wrapper.h"

#include "toolpex/unique_resource.h"

#include <chrono>
#include <thread>
#include <iostream>
#include <future>
#include <ranges>
#include <iterator>

using namespace ::std::chrono_literals;
using namespace koios;

namespace
{
    int basic_count{};
    int dtor_count{};
}

using namespace koios;

void test1()
{
    dtor_count = 0;
    {
        koios::invocable_queue_wrapper iqw{ std_queue_wrapper{} };

        for (size_t i{}; i < 10; ++i)
            iqw.enqueue([h = toolpex::unique_resource(1, [](int){ ++dtor_count; })]{ ++basic_count; });

        koios::invocable_queue_wrapper iqw2{ ::std::move(iqw) };

        for (size_t i{}; i < 5; ++i)
        {
            auto ret = iqw2.dequeue();
            if (ret) (*ret)();
        }

    }
    ::std::cout << ::std::boolalpha << (dtor_count == 10) << ::std::endl;
}

int main()
{
    test1();
    return runtime_exit();
}
