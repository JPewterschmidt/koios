#include "koios/mutex_monitor.h"
#include "koios/coroutine_mutex.h"

#include "spdlog/spdlog.h"

namespace koios
{

void mutex_monitor::add_event(operation op, mutex* m)
{
    ::std::unique_lock lk{ m_lock };
    switch (op)
    {
        case REGISTER: register_mutex(m); break;
        case DEREGISTER: deregister_mutex(m); break;
    }
}

void mutex_monitor::register_mutex(mutex* m)
{
    toolpex_assert(!!m);
    m_mutexes[m] = m_maxid++;
}

void mutex_monitor::deregister_mutex(mutex* m)
{
    toolpex_assert(!!m);
    m_mutexes.erase(m);
}

bool mutex_monitor::empty() const
{
    ::std::shared_lock lk{ m_lock };
    return m_mutexes.empty();
}

void mutex_monitor::print_status() const
{
    ::std::shared_lock lk{ m_lock };
    size_t count{};
    for (auto [m, id] : m_mutexes)
    {
        if (m->be_held())
        {
            ++count;
            spdlog::info("mutex_monitor: No.{} being held", id);
        }
    }
    if (count == m_mutexes.size())
    {
        spdlog::info("all mutexes being held");
    }
}

} // namespace koios
