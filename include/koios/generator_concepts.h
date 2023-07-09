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
    { g.move_next() } -> ::std::same_as<bool>;
    { g.current_value() };
    { g.has_value() } -> ::std::same_as<bool>;
};

template<typename G>
concept ref_result_generator_concept = generator_concept<G> && ::std::is_reference_v<typename G::result_type>;

template<typename G>
concept value_result_generator_concept = generator_concept<G> && !::std::is_reference_v<typename G::result_type>;

KOIOS_NAMESPACE_END

#endif
