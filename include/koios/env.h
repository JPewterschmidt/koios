/* Koios, A c++ async runtime library.
 * Copyright (C) 2023  Jeremy Pewterschmidt
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

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
