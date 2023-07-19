#include "gtest/gtest.h"
#include "koios/thread_pool.h"
#include "koios/task.h"
#include "koios/task_scheduler_concept.h"
#include "koios/runtime.h"

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
    for_basic_test().run_and_get_future().get();
    ASSERT_EQ(flag, 1);

    ASSERT_EQ(for_basic_test2().run_and_get_future().get(), 2);
    
    ASSERT_EQ(referd_obj, 0);
    int& ref = for_basic_test3().run_and_get_future().get();
    ref = 100;
    ASSERT_EQ(referd_obj, 100);
}

TEST(task, sync_task)
{
    ASSERT_EQ(for_sync_task().run_and_get_future().get(), 1);
}

TEST(task, nodiscard)
{
    ASSERT_EQ(for_nodiscard().run_and_get_future().get(), 1);
}
