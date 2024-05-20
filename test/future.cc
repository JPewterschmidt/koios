#include "gtest/gtest.h"
#include "koios/future.h"
#include "koios/future_concepts.h"
#include "koios/task.h"

using namespace koios;

namespace
{
    void func(promise<int> p)
    {
        p.set_value(666);
    }
    int dummy{};
}

TEST(future, basic)
{
    promise<int> p;
    auto f = p.get_future();
    func(::std::move(p));
    ASSERT_EQ(f.get(), 666);
}

TEST(future, nothing_to_get)
{
    try
    {
        promise<int> p;
        auto f = p.get_future();
        f.get_nonblk();
    } 
    catch (const ::std::exception& ex)
    {
        ASSERT_STREQ(ex.what(), "nothing to get.");
    }
}

TEST(future, regular_exception)
{
    try
    {
        promise<int> p;
        auto f = p.get_future();
        p.set_exception(
            ::std::make_exception_ptr(::std::logic_error{"xxx888xxx"})
        );
    } 
    catch (const ::std::exception& ex)
    {
        ASSERT_STREQ(ex.what(), "xxx888xxx");
    }
}

TEST(future, future_ready1)
{
    promise<int> p;
    auto f = p.get_future();
    p.set_value(1);
    ASSERT_TRUE(p.future_ready());
}

TEST(future, future_ready2)
{
    promise<int&> p;
    auto f = p.get_future();
    p.set_value(dummy);
    ASSERT_TRUE(p.future_ready());
}

TEST(future, future_ready3)
{
    promise<void> p;
    auto f = p.get_future();
    p.set_value();
    ASSERT_TRUE(p.future_ready());
}

TEST(future, future_concept)
{
    ASSERT_TRUE(future_concept<koios::future<int>>);
}

namespace 
{

task<int> test_future_aw_func()
{
    co_return 2333;
}

task<bool> test_future_aw()
{
    koios::future<int> fut = test_future_aw_func().run_and_get_future();
    co_return co_await fut.get_async() == 2333;
}
    
} // annoymous namespace

TEST(future, future_aw)
{
    ASSERT_TRUE(test_future_aw().result());
}
