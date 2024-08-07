// This file is part of Koios
// https://github.com/JPewterschmidt/koios
//
// Copyleft 2023 - 2024, ShiXin Wang. All wrongs reserved.

#ifndef KOIOS_IOURING_OP_BATCH_REP_H
#define KOIOS_IOURING_OP_BATCH_REP_H

#include <vector>
#include <memory>
#include <liburing.h>
#include <memory_resource>

#include "toolpex/move_only.h"
#include "toolpex/assert.h"
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
    op_batch_rep(::std::pmr::memory_resource* mr = nullptr)
        : m_mr{ mr ? mr : ::std::pmr::get_default_resource() }, 
          m_sqes(::std::pmr::polymorphic_allocator<::io_uring_sqe>(m_mr)), 
          m_rets(::std::pmr::polymorphic_allocator<ioret_for_any_base>(m_mr))
    {
    }

    op_batch_rep(op_batch_rep&&) noexcept = default;
    op_batch_rep& operator=(op_batch_rep&&) noexcept = default;

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

    void clear() noexcept { m_sqes.clear(); toolpex_assert(!m_rets.empty()); }
    bool empty() const noexcept { return m_sqes.empty(); }

    auto& return_slots() noexcept { return m_rets; }
    const auto& return_slots() const noexcept { return m_rets; }

    void set_user_data(void* userdata);
    void set_user_data(uintptr_t userdata);

    void set_timeout(bool val = true) noexcept { m_was_timeout_set = val; }
    bool was_timeout_set() const noexcept { return m_was_timeout_set; }

private:
    ::std::pmr::memory_resource* m_mr{};
    ::std::vector<::io_uring_sqe, ::std::pmr::polymorphic_allocator<::io_uring_sqe>> m_sqes;
    ::std::vector<ioret_for_any_base, ::std::pmr::polymorphic_allocator<ioret_for_any_base>> m_rets;
    bool m_was_timeout_set{};
};

} // namespace koios::uring

#endif
