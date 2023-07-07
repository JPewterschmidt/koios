#include "gtest/gtest.h"
#include "koios/generator.h"
#include "koios/macros.h"

using namespace koios;

namespace
{
    const ::std::vector<int> ivec{ 1,2,3,4,5 };

    generator<int&> g1()
    {
        for (auto i : ivec) co_yield i;
    }

    generator<int> g2()
    {
        for (int i{};; co_yield i++);
    }
}

TEST(generator, return_ref)
{
    auto g = g1();
    
    g.move_next();
    auto v = g.current_value();
    ASSERT_EQ(v, ivec[0]);

    g.move_next();
    v = g.current_value();
    ASSERT_EQ(v, ivec[1]);

    g.move_next();
    v = g.current_value();
    ASSERT_EQ(v, ivec[2]);
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

