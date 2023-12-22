#ifndef KOIOS_QUEUE_CONCEPTS_H
#define KOIOS_QUEUE_CONCEPTS_H

#include <optional>
#include <concepts>

#include "koios/macros.h"
#include "koios/per_consumer_attr.h"
#include "toolpex/is_specialization_of.h"

KOIOS_NAMESPACE_BEG

template<typename Queue>
concept normal_queue = requires(Queue q)
{
    { q.dequeue() } -> toolpex::is_specialization_of<::std::optional>;
};

template<typename Queue>
concept thread_specific_queue = requires(Queue q)
{
    { q.dequeue(::std::declval<per_consumer_attr>()) } -> toolpex::is_specialization_of<::std::optional>;
};

template<typename Queue>
concept queue_concept = requires(Queue q)
{
    { &Queue::q     };
    { q.empty()     } -> ::std::same_as<bool>;
    { q.size()      } -> ::std::integral;
} && (normal_queue<Queue> || thread_specific_queue<Queue>);

KOIOS_NAMESPACE_END

#endif
