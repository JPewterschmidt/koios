// This file is part of Koios
// https://github.com/JPewterschmidt/koios
//
// Copyleft 2023 - 2024, ShiXin Wang. All wrongs reserved.

#include "koios/waiting_handle.h"
#include "koios/runtime.h"

namespace koios
{

void wake_up(waiting_handle& h)
{
    auto t = ::std::move(h.task);
    [[assume(bool(t))]];
    get_task_scheduler().enqueue(h.attr, ::std::move(t));
}

void wake_up(task_on_the_fly t)
{
    auto& schr = get_task_scheduler();
    schr.enqueue(::std::move(t));
}

} // namespace koios
