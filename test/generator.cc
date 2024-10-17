#include <ranges>

#include "gtest/gtest.h"
#include "koios/generator.h"
#include "koios/task.h"
#include "koios/macros.h"

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
        co_return co_await numbers(10).to<::std::vector>();
    }

    task<::std::vector<lifetimetoy>> test_body_3()
    {
        co_return co_await numbers(10).to<::std::vector<lifetimetoy>>();
    }

    task<::std::vector<lifetimetoy>> test_body_4()
    {
        co_return co_await merge(numbers(5), numbers(4)).to<::std::vector>();
    }

    task<::std::vector<lifetimetoy>> test_body_5()
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
    ASSERT_EQ(test_body_5().result(), rv::iota(0, 10) | rv::transform([](int i){ return lifetimetoy{i}; }) | r::to<::std::vector>());
}

TEST(generator, merge)
{
    ASSERT_EQ(test_body_4().result(), (::std::vector<lifetimetoy>{ 
        lifetimetoy{0},lifetimetoy{0},lifetimetoy{1},lifetimetoy{1},lifetimetoy{2},lifetimetoy{2},lifetimetoy{3},lifetimetoy{3},lifetimetoy{4} 
    }));
}

TEST(generator, over_getting)
{
    ASSERT_TRUE(test_body_6().result());
}
