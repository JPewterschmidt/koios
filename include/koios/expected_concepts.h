#ifndef KOIOS_EXPECTED_CONCEPTS_H
#define KOIOS_EXPECTED_CONCEPTS_H

#include "koios/macros.h"
#include "toolpex/concepts_and_traits.h"
#include "koios/task_concepts.h"

KOIOS_NAMESPACE_BEG

template<typename Exp>
concept regular_expected_like_concept = requires (Exp e)
{
    e.error();
    { e.has_value() } -> ::std::same_as<bool>;
};

template<typename Exp>
concept expected_like_astask_concept = 
    regular_expected_like_concept<typename Exp::value_type>;

template<typename Exp>
concept expected_like_concept = 
    regular_expected_like_concept<Exp> or expected_like_astask_concept<Exp>;

template<typename ExpFunc>
concept expected_callable_concept = expected_like_concept< 
    toolpex::get_return_type_t<ExpFunc> 
>;

KOIOS_NAMESPACE_END

#endif
