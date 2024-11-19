#include "koios/wait_group.h"

namespace koios
{

wait_group_handle wait_group::add(size_t num)
{
    m_count.fetch_add(num, ::std::memory_order_relaxed);
    return { this };
}

void wait_group::wake_up_all()
{
    waiting_handle t;
    while (m_waitings.try_dequeue(t))
    {
        wake_up(t);   
    }
}

} // namespace koios
