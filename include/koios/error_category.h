#ifndef KOIOS_ERROR_CATEGORY_H
#define KOIOS_ERROR_CATEGORY_H

#include "koios/macros.h"

#include <string>
#include <system_error>

KOIOS_NAMESPACE_BEG

enum koios_state_t : int
{
    KOIOS_SUCCEED             = 0,
    KOIOS_EXCEPTION_CATCHED   = 1,
};

enum expected_state_t : int
{
    KOIOS_EXPECTED_SUCCEED             = 0,
    KOIOS_EXPECTED_CANCELED            = 1,
    KOIOS_EXPECTED_EXCEPTION_CATCHED   = 2,
    KOIOS_EXPECTED_USER_DEFINED_ERROR  = 3,
    KOIOS_EXPECTED_NOTHING_TO_GET      = 4,
};

const ::std::error_category& koios_category() noexcept;
const ::std::error_category& expected_category() noexcept;

KOIOS_NAMESPACE_END

#endif
