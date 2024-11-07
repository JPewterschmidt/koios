#ifndef KOIOS_MUTEX_MONITOR_H
#define KOIOS_MUTEX_MONITOR_H

#include <unordered_map>
#include <mutex>
#include <shared_mutex>

#include "koios/per_consumer_attr.h"

namespace koios
{

class mutex;

class mutex_monitor
{
public:
    enum operation
    {
        REGISTER = 0, 
        DEREGISTER = 1,
    };

    mutex_monitor() = default;

    constexpr bool do_occured_nonblk() noexcept { return {}; }
    void add_event(operation op, mutex* m);

    constexpr ::std::chrono::nanoseconds
    max_sleep_duration(const per_consumer_attr& cattr) noexcept
    {
        return ::std::chrono::nanoseconds::max();
    }

    constexpr void quick_stop() noexcept {}
    constexpr void stop() noexcept {}
    constexpr void until_done() const noexcept {}
    constexpr void thread_specific_preparation(const per_consumer_attr& attr) {}
    constexpr bool is_cleanning() const { return {}; }
    constexpr bool done() const { return true; }

    bool empty() const;
    void print_status() const;

private:
    void register_mutex(mutex* m);
    void deregister_mutex(mutex* m);

private:
    mutable ::std::shared_mutex m_lock;
    ::std::unordered_map<koios::mutex*, size_t> m_mutexes;
    size_t m_maxid{};
};

} // namespace koios

#endif
