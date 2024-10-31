// This file is part of Koios
// https://github.com/JPewterschmidt/koios
//
// Copyleft 2023 - 2024, ShiXin Wang. All wrongs reserved.

#ifndef KOIOS_WAITING_HANDLE_H
#define KOIOS_WAITING_HANDLE_H

#include "koios/per_consumer_attr.h"
#include "koios/task_on_the_fly.h"
#include "koios/runtime.h"

namespace koios
{

struct waiting_handle
{
    per_consumer_attr attr;
    task_on_the_fly task;
};

inline void wake_up(waiting_handle& h)
{
    auto t = ::std::move(h.task);
    [[assume(bool(t))]];
    get_task_scheduler().enqueue(h.attr, ::std::move(t));
}

void wake_up(task_on_the_fly f);

} // namespace koios

#endif
