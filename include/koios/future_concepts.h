// This file is part of Koios
// https://github.com/JPewterschmidt/koios
//
// Copyleft 2023 - 2024, ShiXin Wang. All wrongs reserved.

#ifndef KOIOS_FUTURE_CONCEPTS_H
#define KOIOS_FUTURE_CONCEPTS_H

#include "koios/macros.h"
#include "toolpex/concepts_and_traits.h"

KOIOS_NAMESPACE_BEG

template<typename Future>
concept future_concept = requires (Future f)
{
    { f.get() } -> ::std::same_as<typename Future::value_type>;
    { f.get_nonblk() } -> ::std::same_as<typename Future::value_type>;
    { f.ready() } -> toolpex::boolean_testable;
};

KOIOS_NAMESPACE_END

#endif
