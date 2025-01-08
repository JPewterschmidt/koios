#include "gtest/gtest.h"
#include "koios/thread_pool.h"
#include "koios/task.h"
#include "koios/expected.h"
#include "koios/task_scheduler_concept.h"
#include "koios/runtime.h"
#include "koios/this_task.h"
#include "koios/task_release_once.h"
#include "koios/from_result.h"
#include "koios/future.h"

#include <ranges>

using namespace koios;
namespace r = ::std::ranges;
namespace rv = ::std::ranges::views;

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
        const int i = 2;
        co_return i;
    }

    task<int&> for_basic_test3()
    {
        co_return referd_obj;
    }

    nodiscard_task<int> for_nodiscard()
    {
        co_return 1;
    }
}

TEST(task, basic)
{
    flag = 0;
    referd_obj = 0;

    for_basic_test().run_and_get_future().get();
    ASSERT_EQ(flag, 1);

    ASSERT_EQ(for_basic_test2().run_and_get_future().get(), 2);
    
    ASSERT_EQ(referd_obj, 0);
    int& ref = for_basic_test3().run_and_get_future().get();
    ref = 100;
    ASSERT_EQ(referd_obj, 100);
}

TEST(task, nodiscard)
{
    flag = 0;
    referd_obj = 0;
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
    flag2 = flag3 = 0;
    emit_func().result();
    ASSERT_EQ(flag2, 1);
    ASSERT_EQ(flag3, 1);
}

//namespace
//{
//    ::std::error_code ec{ EINVAL, ::std::system_category() };
//
//    expected_task<void, ::std::error_code> expvoid_succeed()
//    {
//        co_return ok();
//    }
//
//    expected_task<void, ::std::error_code> expvoid_failed()
//    {
//        co_return unexpected(ec);
//    }
//
//    lazy_task<bool> emitter_void()
//    {
//        auto ret1 = co_await expvoid_succeed();
//        auto ret2 = co_await expvoid_failed();
//        co_return ret1.has_value() && !ret2.has_value();
//    }
//
//    expected_task<int, ::std::error_code> exp(int i = 0)
//    {
//        co_return i + 1;
//    }
//
//    expected_task<int, ::std::error_code> exp2(int)
//    {
//        co_return unexpected(ec);
//    }
//
//    task<bool> emit_exp_basic()
//    {
//        bool result{ true };
//
//        auto ret = co_await (co_await exp()).and_then(exp);
//        result &= (ret.value() == 2);
//        
//        co_return result;
//    }
//
//    task<bool> emit_failed_exp()
//    {
//        auto ret = co_await (co_await (co_await exp()).and_then(exp2)).and_then(exp);
//        co_return ret.error() == ec && !ret.has_value();
//    }
//
//    task<bool> emit_failed_exp_hasvalue()
//    {
//        auto ret = co_await (co_await (co_await exp()).and_then(exp2)).and_then(exp);
//        co_return ret.has_value();
//    }
//}
//
//TEST(expected_task, basic)
//{
//    ec = ::std::error_code{ EINVAL, ::std::system_category() };
//    ASSERT_TRUE(emit_exp_basic().result());
//    ASSERT_TRUE(emitter_void().result());
//}
//
//TEST(expected_task, failed)
//{
//    ec = ::std::error_code{ EINVAL, ::std::system_category() };
//    ASSERT_TRUE(emit_failed_exp().result());
//    ASSERT_FALSE(emit_failed_exp_hasvalue().result());
//}
//
namespace
{
    bool hascopyed{ false };
    class lifetime
    {
    public:
        lifetime() = default;
        lifetime(const lifetime&) { hascopyed = true; }
        lifetime& operator= (const lifetime&) { hascopyed = true; return *this; }
        lifetime(lifetime&&) noexcept = default;
        lifetime& operator=(lifetime&&) noexcept = default;
    };
    
    task<lifetime> should_not_copy()
    {
        lifetime ret;
        co_return ret;
    }
}

TEST(task, should_not_copy_ret)
{
    hascopyed = false;
    (void)should_not_copy().result();
    ASSERT_FALSE(hascopyed);
}

namespace
{
    task<int> co_await_all1()
    {
        co_return 1;
    }

    task<double> co_await_all2()
    {
        co_return 1.0;
    }

    task<lifetime> co_await_all3()
    {
        lifetime ret;
        co_return ret;
    }

    lazy_task<bool> emit_co_await_all_tests()
    {
        auto [i, d, f] = co_await co_await_all(co_await_all1(), co_await_all2(), co_await_all3());
        (void)i;
        (void)d;
        (void)f;
        co_return hascopyed;
    }
    
    lazy_task<> emit_nothing()
    {
        co_return;
    }

    lazy_task<bool> emit_co_await_all_range()
    {
        ::std::vector<koios::future<int>> ifuts {};
        ifuts.push_back(co_await_all1().run_and_get_future());
        ifuts.push_back(co_await_all1().run_and_get_future());
        ifuts.push_back(co_await_all1().run_and_get_future());
        ifuts.push_back(co_await_all1().run_and_get_future());
        ifuts.push_back(co_await_all1().run_and_get_future());
        ifuts.push_back(co_await_all1().run_and_get_future());

        const size_t ifuts_size = ifuts.size();
        auto ret = co_await co_await_all(::std::move(ifuts));
        co_return ret.size() == ifuts_size && ret[0] == ret[1];
    }

    lazy_task<bool> emit_co_await_all_range_nothing()
    {
        ::std::vector<koios::future<void>> ifuts {};
        ifuts.push_back(emit_nothing().run_and_get_future());
        ifuts.push_back(emit_nothing().run_and_get_future());
        ifuts.push_back(emit_nothing().run_and_get_future());
        ifuts.push_back(emit_nothing().run_and_get_future());
        ifuts.push_back(emit_nothing().run_and_get_future());
        ifuts.push_back(emit_nothing().run_and_get_future());

        co_await co_await_all(::std::move(ifuts));

        co_return true;
    }
}

TEST(task, co_await_all)
{
    ASSERT_FALSE(emit_co_await_all_tests().result());
    ASSERT_TRUE(emit_co_await_all_range().result());
    ASSERT_TRUE(emit_co_await_all_range_nothing().result());
}

namespace
{

lazy_task<bool> emit_yield_test1()
{
    co_await this_task::yield();
    co_return true;
}

task<bool> emit_yield_test2()
{
    co_await this_task::yield();
    co_return true;
}

}

TEST(this_task, yield)
{
    ASSERT_TRUE(emit_yield_test1().result());
    ASSERT_TRUE(emit_yield_test2().result());
    ASSERT_TRUE(make_lazy(emit_yield_test2).result());
}

TEST(task_release_once, basic)
{
    koios::task_on_the_fly t{};
    koios::task_release_once tt{ ::std::move(t) };
    auto ttt = tt.release();
    ASSERT_TRUE(ttt.has_value());
    ttt = tt.release();
    ASSERT_FALSE(ttt.has_value());
}

namespace
{

auto get_a_awaitable()
{
    return from_result(2);
}

task<bool> from_result_test()
{
    co_return 2 == co_await get_a_awaitable();
}

}

TEST(task, from_result)
{
    ASSERT_TRUE(from_result_test().result());   
}
