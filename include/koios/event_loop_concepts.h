#ifndef KOIOS_EVENT_LOOP_CONCEPTS_H
#define KOIOS_EVENT_LOOP_CONCEPTS_H

#include "koios/macros.h"
#include <concepts>

KOIOS_NAMESPACE_BEG

template<typename EL>
concept event_loop_concept = requires(EL e)
{
    e.thread_specific_preparation();
    e.do_occured_nonblk();
    //{ &EL::add_event };
    e.stop();
    e.quick_stop();
    e.until_done();
    e.max_sleep_duration();
} && ::std::default_initializable<EL>;

KOIOS_NAMESPACE_END

#endif
