#ifndef KOIOS_ERROR_CATEGORY_H
#define KOIOS_ERROR_CATEGORY_H

#include "koios/macros.h"

#include <string>
#include <system_error>

KOIOS_NAMESPACE_BEG

const ::std::error_category& expected_category() noexcept;

KOIOS_NAMESPACE_END

#endif
