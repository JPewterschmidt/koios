#include "gtest/gtest.h"
#include "koios/thread_pool.h"
#include "koios/global_task_scheduler.h"
#include "koios/task_scheduler_concept.h"
#include "koios/task.h"

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
    task_scheduler_concept auto& scheduler = koios::get_task_scheduler();
    scheduler.enqueue(starter());
    scheduler.stop();
    ASSERT_EQ(result, 1);
}
