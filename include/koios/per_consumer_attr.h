// This file is part of Koios
// https://github.com/JPewterschmidt/koios
//
// Copyleft 2023 - 2024, ShiXin Wang. All wrongs reserved.

#ifndef KOIOS_PER_CONSUMER_ATTR_H
#define KOIOS_PER_CONSUMER_ATTR_H

#include <thread>

#include "koios/macros.h"

#undef BLOCK_SIZE
#include "concurrentqueue/concurrentqueue.h"

KOIOS_NAMESPACE_BEG

struct per_consumer_attr
{
    ::std::thread::id thread_id{ ::std::this_thread::get_id() };
    ::std::thread::id main_thread_id;
    size_t number_of_threads{};
    moodycamel::ConsumerToken* q_producer_token{};
};

KOIOS_NAMESPACE_END

#endif
