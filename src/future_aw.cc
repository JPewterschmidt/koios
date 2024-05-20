#include "koios/future_aw.h"
#include "koios/runtime.h"

namespace koios::fp_detials
{

void future_aw_detial::awake()
{
    get_task_scheduler().enqueue(::std::move(m_waiting));
}

} // namespace koios::fp_detials
