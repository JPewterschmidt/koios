// This file is part of Koios
// https://github.com/JPewterschmidt/koios
//
// Copyleft 2023 - 2024, ShiXin Wang. All wrongs reserved.

#include "koios/error_category.h"
#include "toolpex/string_ext.h"
#include <unordered_map>
#include "magic_enum.hpp"

KOIOS_NAMESPACE_BEG

using namespace ::std::string_literals;

static const ::std::string koios_ctgr_name{ 
    "koios' error category"
};

static const ::std::string exp_ctgr_name{ 
    "koios expected related facilities' error category"
};

namespace
{
    class koios_category_t : public ::std::error_category
    {
    public:
        virtual const char* name() const noexcept override { return koios_ctgr_name.c_str(); }
        virtual ~koios_category_t() noexcept {}
        virtual ::std::string message(int condition) const override;
    };

    class expected_category_t : public koios_category_t
    {
    public:
        virtual const char* name() const noexcept override { return exp_ctgr_name.c_str(); }
        virtual ~expected_category_t() noexcept {}
        virtual ::std::string message(int condition) const override;
    };

    ::std::string 
    koios_category_t::
    message(int condition) const
    {
        const koios_state_t c = static_cast<koios_state_t>(condition);
        return toolpex::lazy_string_concater{} + koios_ctgr_name + magic_enum::enum_name(c);
    }

    ::std::string 
    expected_category_t::
    message(int condition) const
    {
        const expected_state_t c = static_cast<expected_state_t>(condition);
        return toolpex::lazy_string_concater{} + exp_ctgr_name + magic_enum::enum_name(c);
    }
}

const ::std::error_category& koios_category() noexcept
{
    static const koios::koios_category_t result{};
    return result;
}

const ::std::error_category& expected_category() noexcept
{
    static const koios::expected_category_t result{};
    return result;
}

const ::std::error_code& std_canceled_ec() noexcept
{
    static const ::std::error_code result{ 
        ECANCELED, ::std::system_category() 
    };
    return result;
}

KOIOS_NAMESPACE_END
