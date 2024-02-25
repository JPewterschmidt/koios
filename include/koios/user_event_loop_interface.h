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

#ifndef KOIOS_USER_EVENT_LOOP_INTERFACE_H
#define KOIOS_USER_EVENT_LOOP_INTERFACE_H

#include "koios/macros.h"
#include <memory>

KOIOS_NAMESPACE_BEG

class user_event_loop_interface
{
public:
    using sptr = ::std::shared_ptr<user_event_loop_interface>;

public:
    virtual void thread_specific_preparation(const per_consumer_attr& attr) noexcept = 0;
    virtual void stop() noexcept = 0;
    virtual void quick_stop() noexcept = 0;
    virtual void until_done() = 0;
    virtual ::std::chrono::milliseconds max_sleep_duration(const per_consumer_attr& attr) noexcept = 0;
    virtual void do_occured_nonblk() noexcept = 0;
};

KOIOS_NAMESPACE_END

#endif
