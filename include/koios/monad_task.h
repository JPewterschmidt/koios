#ifndef KOIOS_MONAD_TASK_H
#define KOIOS_MONAD_TASK_H

#include <future>
#include <memory>
#include <variant>
#include <concepts>
#include <system_error>
#include <iostream> // only for a temporarily logging

#include "koios/macros.h"
#include "koios/task.h"
#include "koios/expected_result.h"

#include "toolpex/is_specialization_of.h"
#include "toolpex/exceptions.h"


KOIOS_NAMESPACE_BEG

template<
    typename T, 
    ::std::default_initializable E = ::std::error_code, 
    driver_policy_concept DriverPolicy = run_this_async,
    typename Discardable = discardable>
class [[nodiscard]] monad_task;

template<typename F, typename Arg>
concept monad_transform_task_concept = requires(F f)
{
    { f(::std::declval<Arg>()) } -> toolpex::is_specialization_of<monad_task>;
};

template<
    typename T, 
    ::std::default_initializable E, 
    driver_policy_concept DriverPolicy,
    typename Discardable>
class [[nodiscard]] monad_task
    : public _task<expected_result<T, E>, DriverPolicy, Discardable>::_type
{
public:
    using value_type = T;
    using error_type = E;
    using base_type = _task<expected_result<T, E>, DriverPolicy, Discardable>::_type;
    using promise_type = typename base_type::promise_type;

private:
    using future_type = typename base_type::future_type;

public:
    monad_task(promise_type& p)
        : base_type{ p }
    {
        ::std::cout << "the monad task functionality now is not implemented!" << ::std::endl;
    }

    monad_task(base_type&& t)
        : base_type{ ::std::move(t) }
    {
        ::std::cout << "the monad task functionality now is not implemented!" << ::std::endl;
    }

    monad_task(monad_task&& other) noexcept = default;

public:
    void run()
    {
        auto future = base_type::run_and_get_future();
        m_value_or_error = future.get();
    }

public:
    auto transform(monad_transform_task_concept<value_type> auto&& f)
    {
        run();
        if (!m_value_or_error.has_value())
            toolpex::not_implemented("nonad trasform");
        return f(::std::move(m_value_or_error.get_value()));
    }

private:
    expected_result<value_type, error_type> m_value_or_error;
};

KOIOS_NAMESPACE_END

#endif
