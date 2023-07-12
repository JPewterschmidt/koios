#include "gtest/gtest.h"
#include "koios/thread_pool.h"
#include "koios/task.h"
#include "koios/task_scheduler_concept.h"
#include "koios/global_task_scheduler.h"

using namespace koios;

namespace 
{
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

    sync_task<int> for_sync_task()
    {
        co_return 1;
    }
    
    nodiscard_task<int> for_nodiscard()
    {
        co_return 1;
    }
}

TEST(task, basic)
{
    auto t1 = for_basic_test();
    auto& f1 = t1.future();
    t1.run();
    f1.get();
    ASSERT_EQ(flag, 1);

    auto t2 = for_basic_test2();
    auto& f2 = t2.future();
    t2.run();
    ASSERT_EQ(f2.get(), 2);
    
    ASSERT_EQ(referd_obj, 0);
    auto t3 = for_basic_test3();
    auto& f3 = t3.future();
    t3.run();
    int& ref = f3.get();
    ref = 100;
    ASSERT_EQ(referd_obj, 100);
}

TEST(task, sync_task)
{
    auto t = for_sync_task();
    auto& f = t.future();
    t.run();
    ASSERT_EQ(f.get(), 1);
}

TEST(task, nodiscard)
{
    ASSERT_EQ(for_nodiscard().run_with_future().get(), 1);
}
