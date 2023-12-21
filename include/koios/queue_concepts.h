#ifndef KOIOS_QUEUE_CONCEPTS_H
#define KOIOS_QUEUE_CONCEPTS_H

#include <optional>
#include <concepts>

#include "koios/macros.h"
#include "toolpex/is_specialization_of.h"

KOIOS_NAMESPACE_BEG

template<typename Queue>
concept queue_concept = requires(Queue q)
{
    { &Queue::q     };
    { q.dequeue()   } -> toolpex::is_specialization_of<::std::optional>;
    { q.empty()     } -> ::std::same_as<bool>;
    { q.size()      } -> ::std::integral;
};

KOIOS_NAMESPACE_END

#endif
