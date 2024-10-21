// This file is part of Koios
// https://github.com/JPewterschmidt/koios
//
// Copyleft 2023 - 2024, ShiXin Wang. All wrongs reserved.

#ifndef KOIOS_GENERATOR_CONCEPTS_H
#define KOIOS_GENERATOR_CONCEPTS_H

#include <concepts>
#include <type_traits>

#include "koios/macros.h"
#include "koios/task_concepts.h"

KOIOS_NAMESPACE_BEG

template<typename G>
concept generator_concept = requires(G g)
{
    typename G::promise_type;
    typename G::result_type;
    { g.current_value() };
    { g.has_value() } -> toolpex::boolean_testable;
    { g.next_value_async() } -> awaitible_concept;
};

KOIOS_NAMESPACE_END

#endif
