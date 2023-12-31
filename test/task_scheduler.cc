#include "gtest/gtest.h"
#include "koios/thread_pool.h"
#include "koios/runtime.h"
#include "koios/task_scheduler_concept.h"
#include "koios/task.h"

#include <vector>

using namespace koios;

::std::atomic_int result{};

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
    result += co_await coro();
}

TEST(task_scheduler, basic)
{
    constexpr size_t loop_size{ 10 };
    ::std::vector<koios::future<void>> fvec{};
    for (size_t i{}; i < loop_size; ++i)
        fvec.emplace_back(starter().run_and_get_future());

    for (auto& item : fvec)
        item.get();

    ASSERT_EQ(result, 5 * loop_size);
}
