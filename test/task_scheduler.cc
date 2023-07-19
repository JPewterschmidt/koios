#include "gtest/gtest.h"
#include "koios/thread_pool.h"
#include "koios/runtime.h"
#include "koios/task_scheduler_concept.h"
#include "koios/task.h"
#include <semaphore>

using namespace koios;

int result{};

task<int> coro()
{
    co_return 1;
}

task<void> starter()
{
    result = co_await coro();
}

TEST(task_scheduler, basic)
{
    starter().run_and_get_future().get();
    ASSERT_EQ(result, 1);
}
