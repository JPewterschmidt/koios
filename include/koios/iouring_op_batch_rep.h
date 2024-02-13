#ifndef KOIOS_IOURING_OP_BATCH_REP_H
#define KOIOS_IOURING_OP_BATCH_REP_H

#include <vector>
#include <liburing.h>
#include <cassert>
#include "toolpex/move_only.h"
#include "koios/iouring_ioret.h"

namespace koios::uring
{

class op_batch_rep : public toolpex::move_only
{
public:
    constexpr op_batch_rep() = default;

    ::io_uring_sqe* get_sqe()
    {
        return &m_sqes.emplace_back();
    }

    auto begin()    const noexcept { return m_sqes.begin(); }
    auto end()      const noexcept { return m_sqes.end(); }
    auto& back()          noexcept { return m_sqes.back(); }
    
    void add_ret(int ret, int flags) { m_rets.emplace_back(ret, flags); }
    bool has_enough_ret() const noexcept { return m_rets.size() == m_sqes.size(); }

    void clear() noexcept { m_sqes.clear(); assert(!m_rets.empty()); }
    bool empty() const noexcept { return m_sqes.empty(); }

    auto& return_slots() noexcept { return m_rets; }
    const auto& return_slots() const noexcept { return m_rets; }

    void set_user_data(void* userdata);
    void set_user_data(uint64_t userdata);
    
private:
    ::std::vector<::io_uring_sqe> m_sqes;
    ::std::vector<ioret_for_any_base> m_rets;
};

}

#endif
