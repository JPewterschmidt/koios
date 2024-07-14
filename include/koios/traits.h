// This file is part of Koios
// https://github.com/JPewterschmidt/koios
//
// Copyleft 2023 - 2024, ShiXin Wang. All wrongs reserved.

#ifndef KOIOS_TRAITS_H
#define KOIOS_TRAITS_H

#include "koios/macros.h"
#include "koios/per_consumer_attr.h"

KOIOS_NAMESPACE_BEG

template<typename T>
concept has_thread_specific_preparation = requires (T t)
{
    t.thread_specific_preparation(::std::declval<per_consumer_attr>());
};

KOIOS_NAMESPACE_END

#endif
