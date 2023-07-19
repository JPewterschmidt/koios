#include "gtest/gtest.h"
#include "koios/thread_pool.h"
#include "koios/runtime.h"
#include "koios/task_scheduler_concept.h"
#include "koios/task.h"
#include <semaphore>

using namespace koios;

int result{};
::std::binary_semaphore sem{0}; 

task<int> coro()
{
    co_return 1;
}

task<void> starter()
{
    result = co_await coro();
    sem.release();
}

TEST(task_scheduler, basic)
{
    task_scheduler_concept auto& scheduler = koios::get_task_scheduler();
    scheduler.enqueue(starter());

    sem.acquire();
    ASSERT_EQ(result, 1);
}
