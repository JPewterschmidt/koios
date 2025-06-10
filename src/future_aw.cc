// This file is part of Koios
// https://github.com/JPewterschmidt/koios
//
// Copyleft 2023 - 2025, ShiXin Wang. All wrongs reserved.

#include <string>
#include <span>
#include <cstddef>

#include "koios/future_aw.h"
#include "koios/runtime.h"
#include "koios/future.h"
#include "koios/lite_future.h"

template class koios::future_aw<koios::future<bool>>;
template class koios::future_aw<koios::future<void>>;
template class koios::future_aw<koios::future<size_t>>;
template class koios::future_aw<koios::future<::std::string>>;
template class koios::future_aw<koios::future<::std::span<::std::byte>>>;
template class koios::future_aw<koios::future<::std::span<const ::std::byte>>>;

template class koios::future_aw<koios::lite_future<bool>>;
template class koios::future_aw<koios::lite_future<void>>;
template class koios::future_aw<koios::lite_future<size_t>>;
template class koios::future_aw<koios::lite_future<::std::string>>;
template class koios::future_aw<koios::lite_future<::std::span<::std::byte>>>;
template class koios::future_aw<koios::lite_future<::std::span<const ::std::byte>>>;
