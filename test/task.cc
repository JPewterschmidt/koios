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

namespace
{
    int flag2{};
    int flag3{};
}

task<void> inner_thrower()
{
    throw ::std::runtime_error{"inner thrown"};
}

task<void> exception_test1()
{
    ::std::vector<int> ivec(100, 1);
    co_await inner_thrower();
    ivec.push_back(1);
    co_return;
}

task<int> exception_test2()
{
    throw ::std::runtime_error{"rt err"};
    co_return 1;
}

task<void> emit_func()
{
    try 
    { 
        ::std::vector<int> ivec(100, 1);
        co_await exception_test1(); 
        ivec.push_back(1);
    } 
    catch (...) 
    { 
        flag2 = 1; 
    }
    try { (void) co_await exception_test2(); } catch (...) { flag3 = 1; }
}

TEST(task, exception)
{
    emit_func().result();
    ASSERT_EQ(flag2, 1);
    ASSERT_EQ(flag3, 1);
}
