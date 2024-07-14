// This file is part of Koios
// https://github.com/JPewterschmidt/koios
//
// Copyleft 2023 - 2024, ShiXin Wang. All wrongs reserved.

#ifndef KOIOS_USER_EVENT_LOOP_INTERFACE_H
#define KOIOS_USER_EVENT_LOOP_INTERFACE_H

#include "koios/macros.h"
#include <memory>

KOIOS_NAMESPACE_BEG

class user_event_loop_interface
{
public:
    using sptr = ::std::shared_ptr<user_event_loop_interface>;

public:
    virtual void thread_specific_preparation(const per_consumer_attr& attr) noexcept = 0;
    virtual void stop() noexcept = 0;
    virtual void quick_stop() noexcept = 0;
    virtual void until_done() = 0;
    virtual ::std::chrono::milliseconds max_sleep_duration(const per_consumer_attr& attr) noexcept = 0;
    virtual void do_occured_nonblk() noexcept = 0;
};

KOIOS_NAMESPACE_END

#endif
