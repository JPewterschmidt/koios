#include <ranges>

#include "gtest/gtest.h"
#include "koios/generator.h"
#include "koios/task.h"
#include "koios/macros.h"

using namespace koios;
namespace r = ::std::ranges;
namespace rv = r::views;

//namespace
//{
//    generator<int> g1(int last_val)
//    {
//        for (int i = 1; i <= last_val; ++i)
//            co_yield i;
//    }
//
//    generator<int> g2()
//    {
//        for (int i{};; co_yield i++);
//    }
//}
//
//TEST(generator, return_value)
//{
//    auto g = g2();
//    
//    g.move_next();
//    auto v = g.current_value();
//    ASSERT_EQ(v, 0);
//
//    g.move_next();
//    v = g.current_value();
//    ASSERT_EQ(v, 1);
//
//    g.move_next();
//    v = g.current_value();
//    ASSERT_EQ(v, 2);
//}
//
//TEST(generator_iterator, ranged_for)
//{
//    auto g = g1(10);
//    int count{1};
//    
//    for (const auto& i : g)
//        ASSERT_EQ(count++, i);
//}
//
//TEST(generator_iterator, multi_dereference)
//{
//    auto g = g1(10);
//
//    int val1{}, val2{};
//    for (auto iter = g.begin(); iter != g.end(); ++iter)
//    {
//        val1 = *iter;
//        val2 = *iter;
//
//        ASSERT_EQ(val1, val2);
//    }
//}
    
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
    generator<int> numbers(int until)
    {
        for (int i{}; i < until; ++i)
            co_yield i;
    }

    task<::std::vector<int>> test_body_2()
    {
        co_return co_await numbers(10).to<::std::vector>();
    }

    task<::std::vector<int>> test_body_3()
    {
        co_return co_await numbers(10).to<::std::vector<int>>();
    }

    task<::std::vector<int>> test_body_4()
    {
        co_return co_await merge(numbers(5), numbers(4)).to<::std::vector>();
    }

    task<::std::vector<int>> test_body_5()
    {
        ::std::vector<int> result;
        co_await numbers(10).to(::std::back_inserter(result));
        co_return result;
    }

    task<bool> test_body_6()
    {
        auto g = numbers(10);
        ::std::optional<int> opt;
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
    ASSERT_EQ(test_body_2().result(), rv::iota(0, 10) | r::to<::std::vector>());
    ASSERT_EQ(test_body_3().result(), rv::iota(0, 10) | r::to<::std::vector>());
    ASSERT_EQ(test_body_5().result(), rv::iota(0, 10) | r::to<::std::vector>());
}

TEST(generator, merge)
{
    ASSERT_EQ(test_body_4().result(), (::std::vector{ 0,0,1,1,2,2,3,3,4 }));
}

TEST(generator, over_getting)
{
    ASSERT_TRUE(test_body_6().result());
}
