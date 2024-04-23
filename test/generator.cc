#include <ranges>

#include "gtest/gtest.h"
#include "koios/generator.h"
#include "koios/macros.h"

using namespace koios;

namespace
{
    generator<int> g1(int last_val)
    {
        for (int i = 1; i <= last_val; ++i)
            co_yield i;
    }

    generator<int> g2()
    {
        for (int i{};; co_yield i++);
    }
}

TEST(generator, return_value)
{
    auto g = g2();
    
    g.move_next();
    auto v = g.current_value();
    ASSERT_EQ(v, 0);

    g.move_next();
    v = g.current_value();
    ASSERT_EQ(v, 1);

    g.move_next();
    v = g.current_value();
    ASSERT_EQ(v, 2);
}

TEST(generator_iterator, ranged_for)
{
    auto g = g1(10);
    int count{1};
    
    for (const auto& i : g)
        ASSERT_EQ(count++, i);
}

TEST(generator_iterator, multi_dereference)
{
    auto g = g1(10);

    int val1{}, val2{};
    for (auto iter = g.begin(); iter != g.end(); ++iter)
    {
        val1 = *iter;
        val2 = *iter;

        ASSERT_EQ(val1, val2);
    }
}
