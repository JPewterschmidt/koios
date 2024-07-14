// This file is part of Koios
// https://github.com/JPewterschmidt/koios
//
// Copyleft 2023 - 2024, ShiXin Wang. All wrongs reserved.


#include "koios/future_aw.h"
#include "koios/runtime.h"

namespace koios::fp_detials
{

void future_aw_detial::awake()
{
    get_task_scheduler().enqueue(::std::move(m_waiting));
}

} // namespace koios::fp_detials
