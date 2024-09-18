// This file is part of Koios
// https://github.com/JPewterschmidt/koios
//
// Copyleft 2023 - 2024, ShiXin Wang. All wrongs reserved.

#ifndef KOIOS_UTILITY_H
#define KOIOS_UTILITY_H

#include <variant>

#include "koios/macros.h"
#include "koios/functional.h"

KOIOS_NAMESPACE_BEG

/**
 * This function will convert a runtime bool value into a variant, 
 * which could be visited as `if constexpr`. 
 * Very usefual in a tight loop which evaluates 
 * a bool expression which value never changed, as a configuration.
 * There's a usecase in source code. which deserve a shot.
 */
inline ::std::variant<::std::true_type, ::std::false_type>
to_bool_variant(bool val)
{
    /*   USE CASE: 
     *   ::std::visit([&](auto boolvar) mutable
     *   { 
     *       // do something
     *       for (...)
     *       {
     *           if constexpr (boolvar)
     *           {
     *               // do something
     *           }
     *       }
     *   }, boolvar_to_constant(slience));
     */
    
    if (val) return { ::std::true_type{} };
    else     return { ::std::false_type{} };
}

KOIOS_NAMESPACE_END

#endif
