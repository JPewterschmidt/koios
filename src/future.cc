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

#include "koios/future.h"

#include <string>
#include <string_view>
#include <span>
#include <cstddef>

template class koios::promise<int>;
template class koios::promise<size_t>;
template class koios::promise<void>;
template class koios::promise<bool>;
template class koios::promise<::std::string>;
template class koios::promise<::std::string_view>;
template class koios::promise<::std::span<unsigned char>>;
template class koios::promise<::std::span<::std::byte>>;

template class koios::future<int>;
template class koios::future<size_t>;
template class koios::future<void>;
template class koios::future<bool>;
template class koios::future<::std::string>;
template class koios::future<::std::string_view>;
template class koios::future<::std::span<unsigned char>>;
template class koios::future<::std::span<::std::byte>>;
