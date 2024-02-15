#ifndef KOIOS_IOURING_OP_BATCH_EXECUTE_AW_H
#define KOIOS_IOURING_OP_BATCH_EXECUTE_AW_H

#include <vector>
#include "koios/task_on_the_fly.h"
#include "koios/iouring_ioret.h"

namespace koios::uring
{

class op_batch_rep;
class op_batch_execute_aw
{
public:
    op_batch_execute_aw(op_batch_rep& rep) noexcept
        : m_rep{ &rep }
    {
    }

    constexpr bool await_ready() const noexcept { return false; }
    void await_suspend(task_on_the_fly t);
    constexpr void await_resume() const noexcept {}

private:
    op_batch_rep* m_rep;      
};

} // namespace koios::uring

#endif
