#ifndef KOIOS_EVENT_LOOP_H
#define KOIOS_EVENT_LOOP_H

#include <concepts>

#include "koios/macros.h"

KOIOS_NAMESPACE_BEG

template<::std::default_initializable... Loops>
class event_loop : private Loops...
{
public:
    void do_occured_nonblk() noexcept
    {
        (Loops::do_occured_nonblk(), ...);
    }

    template<typename SpecificLoop>
    auto& as() noexcept
    {
        return static_cast<SpecificLoop&>(*this);
    }

    template<typename SpecificLoop>
    void add_event(auto&& data)
    {
        SpecificLoop::add_event(::std::forward<decltype(data)>(data));
    }

protected:
    event_loop(event_loop&&) noexcept = default;
    event_loop& operator=(event_loop&&) noexcept = default;
};

KOIOS_NAMESPACE_END

#endif
