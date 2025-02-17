// This file is part of Koios
// https://github.com/JPewterschmidt/koios
//
// Copyleft 2023 - 2024, ShiXin Wang. All wrongs reserved.

#ifndef KOIOS_TASK_SCHEDULER_CONCEPT_H
#define KOIOS_TASK_SCHEDULER_CONCEPT_H

#include <concepts>
#include <coroutine>
#include <type_traits>

#include "koios/macros.h"
#include "koios/task_on_the_fly.h"

KOIOS_NAMESPACE_BEG

template<typename TS>
concept task_scheduler_concept = requires(TS ts)
{
    //{ ts.enqueue(::std::declval<::std::coroutine_handle<>>()) };
    { ts.enqueue(::std::declval<koios::task_on_the_fly>()) };
};

KOIOS_NAMESPACE_END

#endif
