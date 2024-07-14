// This file is part of Koios
// https://github.com/JPewterschmidt/koios
//
// Copyleft 2023 - 2024, ShiXin Wang. All wrongs reserved.

#include "koios/get_id_aw.h"
#include "koios/runtime.h"

KOIOS_NAMESPACE_BEG

void get_id_aw::await_suspend(task_on_the_fly h) noexcept
{
    m_result = h.address();
    get_task_scheduler().enqueue(::std::move(h));
}

KOIOS_NAMESPACE_END
