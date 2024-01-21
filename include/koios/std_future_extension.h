/* Koios, A c++ async runtime library.
 * Copyright (C) 2024  Jeremy Pewterschmidt
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

#ifndef KOIOS_FUTURE_H
#define KOIOS_FUTURE_H

#include <chrono>
#include <future>

#include "koios/macros.h"

KOIOS_NAMESPACE_BEG

template<typename T>
class future_extension : public ::std::future<T>
{
public:
    future_extension(::std::future<T>&& stdf) noexcept
        : ::std::future<T>{ ::std::move(stdf) }
    {
    }

    bool ready()
    {
        switch (this->wait_for(::std::chrono::nanoseconds{0}))
        {
            case ::std::future_status::ready: return true;
        }
        return false;
    }
};

KOIOS_NAMESPACE_END

#endif
