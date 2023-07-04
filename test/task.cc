#include "gtest/gtest.h"
#include "koios/thread_pool.h"
#include "koios/task.h"
#include "koios/task_scheduler_concept.h"
#include "koios/global_task_scheduler.h"

using namespace koios;

int flag{};
int referd_obj{};

task<void> for_basic_test()
{
    flag = 1;
    co_return;
}

task<int> for_basic_test2()
{
    co_return 2;
}

task<int&> for_basic_test3()
{
    co_return referd_obj;
}

TEST(task, basic)
{
    auto t = for_basic_test();
    t();
    ASSERT_EQ(flag, 1);

    auto t2 = for_basic_test2();
    auto& f = t2.future();
    t2();
    ASSERT_EQ(f.get(), 2);
    
    ASSERT_EQ(referd_obj, 0);
    auto t3 = for_basic_test3();
    auto& f2 = t3.future();
    t3();
    int& ref = f2.get();
    ref = 100;
    ASSERT_EQ(referd_obj, 100);
}

namespace 
{
    int result{};
    int count{};
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
}

TEST(task, with_scheduler)
{
    task_scheduler_concept auto& scheduler = koios::get_task_scheduler();
    scheduler.enqueue(starter());

    sem.acquire();
    ASSERT_EQ(result, 10);
}

TEST(task, run_async)
{
    result = 0;
    count = 0;
    starter().run_async();
    sem.acquire();
    ASSERT_EQ(result, 10);
}

