#ifndef KOIOS_ENV_H
#define KOIOS_ENV_H

#include "koios/macros.h"

KOIOS_NAMESPACE_BEG

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
