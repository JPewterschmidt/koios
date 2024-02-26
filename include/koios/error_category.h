/* Koios, A c++ async runtime library.
 * Copyright (C) 2023  Jeremy Pewterschmidt
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

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

/*! \brief Similary to `::std::system_category`.*/
const ::std::error_category& koios_category() noexcept;
/*! \brief Similary to `::std::system_category`.*/
const ::std::error_category& expected_category() noexcept;
const ::std::error_code& std_canceled_ec() noexcept;

inline bool is_timeout_ec(const ::std::error_code& ec) noexcept
{
    return ec.value() == ETIME && ec.category() == ::std::system_category();
}

KOIOS_NAMESPACE_END

#endif
