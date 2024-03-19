#ifndef KOIOS_WAITING_HANDLE_H
#define KOIOS_WAITING_HANDLE_H

namespace koios
{

struct waiting_handle
{
    per_consumer_attr attr;
    task_on_the_fly task;
};

} // namespace koios

#endif
