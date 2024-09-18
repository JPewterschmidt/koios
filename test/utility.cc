#include "gtest/gtest.h"
#include "koios/utility.h"

using namespace koios;

TEST(utility, bool_variant)
{
    bool result = ::std::visit([](auto boolvar) {
        if constexpr (boolvar)
        {
            return true;
        }
        return false;
    }, to_bool_variant(true));

    ASSERT_TRUE(result);
}
