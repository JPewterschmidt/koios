#include "gtest/gtest.h"
#include "koios/lite_future.h"

using namespace koios;

TEST(lite_future, basic)
{
    lite_future<int> ifut;
    auto p = ifut.get_promise();
    p.set_value(1);
    ASSERT_TRUE(ifut.ready());
    auto i = ifut.get();
    ASSERT_EQ(i, 1);
}
