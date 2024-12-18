// This file is part of Koios
// https://github.com/JPewterschmidt/koios
//
// Copyleft 2023 - 2024, ShiXin Wang. All wrongs reserved.

#ifndef KOIOS_EVENT_LOOP_CONCEPTS_H
#define KOIOS_EVENT_LOOP_CONCEPTS_H

#include "koios/macros.h"
#include "koios/per_consumer_attr.h"
#include "toolpex/concepts_and_traits.h"
#include <concepts>
#include <utility>
#include <chrono>

KOIOS_NAMESPACE_BEG

template<typename EL>
concept event_loop_concept = requires(EL e)
{
    { &EL::thread_specific_preparation };
    { e.do_occured_nonblk() } -> toolpex::boolean_testable;
    //{ &EL::add_event };
    e.stop();
    e.quick_stop();
    e.until_done();
    e.print_status();
    { e.max_sleep_duration(::std::declval<per_consumer_attr>()) } 
        -> toolpex::is_specialization_of<::std::chrono::duration>;
    { e.empty() } -> toolpex::boolean_testable;
} && ::std::default_initializable<EL>;

KOIOS_NAMESPACE_END

#endif
