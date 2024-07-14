// This file is part of Koios
// https://github.com/JPewterschmidt/koios
//
// Copyleft 2023 - 2024, ShiXin Wang. All wrongs reserved.

#ifndef KOIOS_QUEUE_CONCEPTS_H
#define KOIOS_QUEUE_CONCEPTS_H

#include <optional>
#include <concepts>

#include "koios/macros.h"
#include "koios/per_consumer_attr.h"
#include "toolpex/concepts_and_traits.h"

KOIOS_NAMESPACE_BEG

template<typename Queue>
concept base_queue = requires(Queue q)
{
    //{ &Queue::enqueue };  // overloading 
    { q.empty()     } -> toolpex::boolean_testable;
    { q.size()      } -> ::std::integral;
};

template<typename Queue>
concept normal_queue_concept = requires(Queue q)
{
    { q.dequeue() } -> toolpex::is_specialization_of<::std::optional>;
} && base_queue<Queue>;

template<typename Queue>
concept thread_specific_queue_concept = requires(Queue q)
{
    { q.dequeue(::std::declval<per_consumer_attr>()) } -> toolpex::is_specialization_of<::std::optional>;
} && base_queue<Queue>;

template<typename Queue>
concept queue_concept = normal_queue_concept<Queue> || thread_specific_queue_concept<Queue>;

template<typename Queue>
concept invocable_queue_concept = 
       ::std::invocable<typename Queue::invocable_type> 
    && queue_concept<Queue>;

KOIOS_NAMESPACE_END

#endif
