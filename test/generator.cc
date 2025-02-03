#include <ranges>

#include "gtest/gtest.h"
#include "koios/generator.h"
#include "koios/task.h"
#include "koios/macros.h"
#include "koios/iouring_op_functions.h"

#include "toolpex/lifetimetoy.h"

using namespace koios;
using namespace toolpex;
namespace r = ::std::ranges;
namespace rv = r::views;
    
namespace 
{
    constinit int scale{10};

    lazy_task<int> func()
    {
        co_return 1;
    }
    
    generator<int> gen()
    {
        for (int i{}; i < scale; ++i)
        {
            co_yield co_await func() + i; 
        }
        co_return;
    }

    lazy_task<::std::vector<int>> test_body_1()
    {
        ::std::vector<int> result{};
        auto g = gen();
        ::std::optional<int> iopt;
        while ((iopt = co_await g.next_value_async()))
        {
            result.push_back(*iopt);
        }

        co_return result;
    }
}

TEST(generator, basic)
{
    ASSERT_EQ(
        test_body_1().result(), 
        rv::iota(0, 10) 
            | rv::transform([](int i){ return i + 1; }) 
            | r::to<::std::vector<int>>()
    );
}

namespace 
{
    generator<lifetimetoy> numbers(int until)
    {
        for (int i{}; i < until; ++i)
            co_yield i;
    }

    task<::std::vector<lifetimetoy>> test_body_2()
    {
        auto g = numbers(10);
        co_return co_await g.to<::std::vector>();
    }

    task<::std::vector<lifetimetoy>> test_body_3()
    {
        auto g = numbers(10);
        co_return co_await g.to<::std::vector<lifetimetoy>>();
    }

    task<::std::vector<lifetimetoy>> test_body_3_1()
    {
        co_return co_await numbers(10).to<::std::vector<lifetimetoy>>();
    }

    task<::std::vector<lifetimetoy>> test_body_3_2()
    {
        co_return co_await numbers(10).to<::std::vector>();
    }

    task<::std::vector<lifetimetoy>> test_body_4()
    {
        auto mg = merge(numbers(5), numbers(4));
        co_return co_await mg.to<::std::vector>();
    }

    task<::std::vector<lifetimetoy>> test_body_4_1()
    {
        auto mg = merge(numbers(5), numbers(4)).unique();
        co_return co_await mg.to<::std::vector>();
    }

    task<::std::vector<lifetimetoy>> test_body_5()
    {
        ::std::vector<lifetimetoy> result;
        auto g = numbers(10);
        co_await g.to(::std::back_inserter(result));
        co_return result;
    }

    task<::std::vector<lifetimetoy>> test_body_5_1()
    {
        ::std::vector<lifetimetoy> result;
        co_await numbers(10).to(::std::back_inserter(result));
        co_return result;
    }

    task<bool> test_body_6()
    {
        auto g = numbers(10);
        ::std::optional<lifetimetoy> opt;
        for (int i{}; i < 10; ++i)
            opt = co_await g.next_value_async();

        for (int i{}; i < 10; ++i)
        {
            opt = co_await g.next_value_async();
            if (opt.has_value())
                co_return false;
        }

        co_return true;
    }
}

TEST(generator, to)
{
    ASSERT_EQ(test_body_2().result(), rv::iota(0, 10) | rv::transform([](int i){ return lifetimetoy{i}; }) | r::to<::std::vector>());
    ASSERT_EQ(test_body_3().result(), rv::iota(0, 10) | rv::transform([](int i){ return lifetimetoy{i}; }) | r::to<::std::vector>());
    ASSERT_EQ(test_body_3_1().result(), rv::iota(0, 10) | rv::transform([](int i){ return lifetimetoy{i}; }) | r::to<::std::vector>());
    ASSERT_EQ(test_body_3_2().result(), rv::iota(0, 10) | rv::transform([](int i){ return lifetimetoy{i}; }) | r::to<::std::vector>());
    ASSERT_EQ(test_body_5().result(), rv::iota(0, 10) | rv::transform([](int i){ return lifetimetoy{i}; }) | r::to<::std::vector>());
    ASSERT_EQ(test_body_5_1().result(), rv::iota(0, 10) | rv::transform([](int i){ return lifetimetoy{i}; }) | r::to<::std::vector>());
}

TEST(generator, merge)
{
    ASSERT_EQ(test_body_4().result(), (::std::vector<lifetimetoy>{ 
        lifetimetoy{0},lifetimetoy{0},lifetimetoy{1},lifetimetoy{1},lifetimetoy{2},lifetimetoy{2},lifetimetoy{3},lifetimetoy{3},lifetimetoy{4} 
    }));
}

TEST(generator, unique)
{
    ASSERT_EQ(test_body_4_1().result(), (::std::vector<lifetimetoy>{ 
        lifetimetoy{0},
        lifetimetoy{1},
        lifetimetoy{2},
        lifetimetoy{3},
        lifetimetoy{4} 
    }));
}

TEST(generator, over_getting)
{
    ASSERT_TRUE(test_body_6().result());
}

namespace
{
    task<int> inner_t(int i)
    {
        co_await uring::nop();
        co_return i;
    }

    generator<int> g_t()
    {
        for (int i{}; i < 10; ++i)
        {
            co_yield co_await inner_t(i);
            co_await uring::nop();
        }
        co_await uring::nop();

        co_yield 0;
    }

    task<::std::vector<int>> g_t_test_body()
    {
        auto g = g_t();
        co_return co_await g.to<::std::vector>();
    }
}

TEST(generator, g_t)
{
    ASSERT_EQ(g_t_test_body().result(), (::std::vector<int>{ 0,1,2,3,4,5,6,7,8,9,0 }));
}

namespace
{
    bool generator_lifetime_success{};
    generator<int> lifetime_generator_test()
    {
        struct true_setter
        {
            ~true_setter() noexcept { generator_lifetime_success = true; }
        } _;

        for (int i{}; i < 10; ++i)
            co_yield i;
    }
    
    lazy_task<> half_way_consumer()
    {
        auto gen = lifetime_generator_test();
        (void) co_await gen.next_value_async();
    }
}

TEST(generator, lifetime)
{
    half_way_consumer().result();
    ASSERT_TRUE(generator_lifetime_success);
}

