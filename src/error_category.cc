#include "koios/error_category.h"
#include <unordered_map>
#include <string_view>

KOIOS_NAMESPACE_BEG

using namespace ::std::string_view_literals;
using namespace ::std::string_literals;

static constexpr ::std::string_view categoryname{ 
    "koios expected related facilities' error category"
};

static ::std::unordered_map<int, ::std::string_view> condition_str
{
    { KOIOS_EXPECTED_SUCCEED,           "Succeed"sv },
    { KOIOS_EXPECTED_CANCELED,          "Canceled"sv },
    { KOIOS_EXPECTED_USER_DEFINED_ERROR,"Exception Catched"sv },
};

const char* 
expected_category::
name() const noexcept 
{
    return categoryname.data();
}

::std::string 
expected_category::
message(int condition) const
{
    if (auto found = condition_str.find(condition); 
            found == condition_str.end())
    {
        return ::std::string{categoryname} + ": "s + ::std::string{found->second};
    }
    return ::std::string{categoryname} + ": unkonw"s;
}

KOIOS_NAMESPACE_END
