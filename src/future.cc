// This file is part of Koios
// https://github.com/JPewterschmidt/koios
//
// Copyleft 2023 - 2024, ShiXin Wang. All wrongs reserved.

#include "koios/future.h"
#include "koios/runtime.h"

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
