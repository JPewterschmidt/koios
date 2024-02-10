/* koios, a c++ async runtime library.
 * copyright (c) 2024  jeremy pewterschmidt
 * 
 * this program is free software; you can redistribute it and/or
 * modify it under the terms of the gnu general public license
 * as published by the free software foundation; either version 2
 * of the license, or (at your option) any later version.
 * 
 * this program is distributed in the hope that it will be useful,
 * but without any warranty; without even the implied warranty of
 * merchantability or fitness for a particular purpose.  see the
 * gnu general public license for more details.
 * 
 * you should have received a copy of the gnu general public license
 * along with this program; if not, write to the free software
 * foundation, inc., 51 franklin street, fifth floor, boston, ma  02110-1301, usa.
 */

#include "koios/error_category.h"
#include <unordered_map>

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
        switch (condition)
        {
        case koios_state_t::KOIOS_SUCCEED            : return koios_ctgr_name + " Success";
        case koios_state_t::KOIOS_EXCEPTION_CATCHED  : return koios_ctgr_name + " Exception Catched";
        }
        
        return exp_ctgr_name + " Unknow";
    }

    ::std::string 
    expected_category_t::
    message(int condition) const
    {
        switch (condition)
        {
        case expected_state_t::KOIOS_EXPECTED_SUCCEED            : return exp_ctgr_name + " Success";
        case expected_state_t::KOIOS_EXPECTED_CANCELED           : return exp_ctgr_name + " Canceled";
        case expected_state_t::KOIOS_EXPECTED_EXCEPTION_CATCHED  : return exp_ctgr_name + " Exception Catched";
        case expected_state_t::KOIOS_EXPECTED_USER_DEFINED_ERROR : return exp_ctgr_name + " User Definded Error";
        case expected_state_t::KOIOS_EXPECTED_NOTHING_TO_GET     : return exp_ctgr_name + " Nothing to Get";
        }
        
        return exp_ctgr_name + " Unknow";
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
