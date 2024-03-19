#ifndef KOIOS_WAITING_HANDLE_H
#define KOIOS_WAITING_HANDLE_H

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

} // namespace koios

#endif
