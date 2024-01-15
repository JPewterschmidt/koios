#include "gtest/gtest.h"
#include "koios/utility.h"

using namespace koios;

namespace
{
    task<void> emitter()
    {
        int i = co_await identity(1);
        (void)i;
    }
}

TEST(utility, identity)
{
    emitter().result();
    ASSERT_TRUE(true);
}
