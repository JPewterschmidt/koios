// This file is part of Koios
// https://github.com/JPewterschmidt/koios
//
// Copyleft 2023 - 2024, ShiXin Wang. All wrongs reserved.

#ifndef KOIOS_GENERATOR_CONCEPTS_H
#define KOIOS_GENERATOR_CONCEPTS_H

#include <concepts>
#include <type_traits>

#include "koios/macros.h"

KOIOS_NAMESPACE_BEG

template<typename G>
concept generator_concept = requires(G g)
{
    typename G::promise_type;
    { g.move_next() } -> toolpex::boolean_testable;
    { g.current_value() };
    { g.has_value() } -> toolpex::boolean_testable;
};

KOIOS_NAMESPACE_END

#endif
