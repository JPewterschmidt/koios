#include "gtest/gtest.h"
#include "koios/utility.h"

using namespace koios;

TEST(utility, from_result)
{
    ASSERT_EQ(1, from_result(1).result());
}
