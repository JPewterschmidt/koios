#include "koios/error_category.h"
#include <unordered_map>
#include <string_view>

KOIOS_NAMESPACE_BEG

using namespace ::std::string_view_literals;
using namespace ::std::string_literals;

static constexpr ::std::string_view categoryname{ 
    "koios expected related facilities' error category"
};

namespace
{
    class expected_category_t : public ::std::error_category
    {
    public:
        virtual const char* name() const noexcept override;
        virtual ~expected_category_t() noexcept {}
        virtual ::std::string message(int condition) const override;
    };

    static ::std::unordered_map<int, ::std::string_view> condition_str
    {
        { KOIOS_EXPECTED_SUCCEED,           "Succeed"sv },
        { KOIOS_EXPECTED_CANCELED,          "Canceled"sv },
        { KOIOS_EXPECTED_USER_DEFINED_ERROR,"Exception Catched"sv },
        { KOIOS_EXPECTED_NOTHING_TO_GET,    "Nothing to get"sv },
    };

    const char* 
    expected_category_t::
    name() const noexcept 
    {
        return categoryname.data();
    }

    ::std::string 
    expected_category_t::
    message(int condition) const
    {
        if (auto found = condition_str.find(condition); 
                found == condition_str.end())
        {
            return ::std::string{categoryname} + ": "s + ::std::string{found->second};
        }
        return ::std::string{categoryname} + ": unkonw"s;
    }
}

const ::std::error_category& expected_category() noexcept
{
    static koios::expected_category_t result{};
    return result;
}

KOIOS_NAMESPACE_END
