/* Koios, A c++ async runtime library.
 * Copyright (C) 2024  Jeremy Pewterschmidt
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

#ifndef KOIOS_IOURING_OP_BATCH_REP_H
#define KOIOS_IOURING_OP_BATCH_REP_H

#include <vector>
#include <liburing.h>
#include <cassert>
#include "toolpex/move_only.h"
#include "koios/iouring_ioret.h"

namespace koios::uring
{

/*! \brief  The op_batch representation, all the `io_uring_sqe` will be stored here.
 *  
 *  After operations return, the iouring event loop 
 *  will add those return value and flags back here.
 */
class op_batch_rep : public toolpex::move_only
{
public:
    constexpr op_batch_rep() = default;

    /*! \brief Get the pointer to the next new writable sqe.
     *  
     *  This function will push back a default 
     *  initialized `::io_uring_sqe` object, the return the pointer to it.
     *  But if there's a timeout sqe, 
     *  this function will insert a default initialized `::io_uring_sqe` 
     *  before the timeout request entry (the last one), 
     *  to make sure the last entry is the timeout entry.
     *  If you write a timeout entry, you need call 
     *  the member function `set_timeout(true)` before the next call to `get_sqe()`.
     *
     *  \return The pointer to a new default initialized sqe object.
     */
    ::io_uring_sqe* get_sqe();

    auto begin()    const noexcept { return m_sqes.begin(); }
    auto end()      const noexcept { return m_sqes.end(); }
    auto& back()          noexcept { return m_sqes.back(); }
    
    /*! \brief Add a new return value and flags.
     *
     *  Same order as the sqes, the one of timeout will be stored at the end.
     */
    void add_ret(int ret, int flags) { m_rets.emplace_back(ret, flags); }
    bool has_enough_ret() const noexcept { return m_rets.size() == m_sqes.size(); }

    void clear() noexcept { m_sqes.clear(); assert(!m_rets.empty()); }
    bool empty() const noexcept { return m_sqes.empty(); }

    auto& return_slots() noexcept { return m_rets; }
    const auto& return_slots() const noexcept { return m_rets; }

    void set_user_data(void* userdata);
    void set_user_data(uintptr_t userdata);
    void set_timeout(bool val = true) noexcept { m_was_timeout_set = val; }
    bool was_timeout_set() const noexcept { return m_was_timeout_set; }

private:
    ::std::vector<::io_uring_sqe> m_sqes;
    ::std::vector<ioret_for_any_base> m_rets;
    bool m_was_timeout_set{};
};

}

#endif
