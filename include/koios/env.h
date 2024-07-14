// This file is part of Koios
// https://github.com/JPewterschmidt/koios
//
// Copyleft 2023 - 2024, ShiXin Wang. All wrongs reserved.

#ifndef KOIOS_ENV_H
#define KOIOS_ENV_H

#include "koios/macros.h"

KOIOS_NAMESPACE_BEG

/*! \brief check wether the compiling mode is profiling mode.
 *
 *  Koios would be hinted by this to reduce some uncertain observation result.
 *  For example, the thread_pool would evaluate the `max_sleep_duration`, 
 *  this time value are uncertain each called.
 *  Would probably diturb the profiling result.
 */
inline consteval bool is_profiling_mode()
{
#ifdef KOIOS_PROFILING
    return true;
#else
    return false;
#endif
}

KOIOS_NAMESPACE_END

#endif
