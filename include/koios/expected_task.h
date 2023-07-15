#ifndef KOIOS_EXPECTED_TASK_H
#define KOIOS_EXPECTED_TASK_H

#include <future>
#include <memory>
#include <variant>
#include <concepts>

#include "koios/macros.h"
#include "koios/task.h"
#include "toolpex/is_specialization_of.h"
#include "toolpex/exceptions.h"

KOIOS_NAMESPACE_BEG

template<typename T, ::std::default_initializable E>
class expected_result
{
private:
    template<typename, ::std::default_initializable, driver_policy_concept, typename> 
    friend class expected_task;

    expected_result() = default;

public:
    using value_type = T;
    using error_type = E;
    using value_storage = ::std::unique_ptr<value_type>;

    expected_result(value_type val)
        : m_result{ ::std::make_unique<value_type>(::std::move(val)) }
    {
    }

    explicit expected_result(error_type e)
        : m_error{ ::std::move(e) }
    {
    }

    const error_type& error() const noexcept { return m_error; }
    bool has_value() const noexcept { return m_result != nullptr; }
    operator bool() const noexcept { return has_value(); }
    value_storage& value() noexcept { return m_result; }
    value_type get_value() { return ::std::move(*value()); }
    
private:
    value_storage m_result;
    error_type m_error{};
};

template<
    typename T, 
    ::std::default_initializable E, 
    driver_policy_concept DriverPolicy,
    typename Discardable>
class [[nodiscard]] expected_task;

template<typename F, typename Arg>
concept monad_transform_task_concept = requires(F f)
{
    { f(::std::declval<Arg>()) } -> toolpex::is_specialization_of<expected_task>;
};

template<
    typename T, 
    ::std::default_initializable E, 
    driver_policy_concept DriverPolicy,
    typename Discardable>
class [[nodiscard]] expected_task
    : public _task<expected_result<T, E>, DriverPolicy, Discardable>::_type
{
public:
    using value_type = T;
    using error_type = E;
    using base_type = _task<T, DriverPolicy, Discardable>::_type;
    using promise_type = typename base_type::promise_type;

private:
    using future_type = typename base_type::future_type;

public:
    expected_task(promise_type& p)
        : base_type{ p }
    {
    }

    expected_task(base_type&& t)
        : base_type{ ::std::move(t) }
    {
    }

    expected_task(expected_task&& other) noexcept = default;

    future_type get_future() = delete;
    future_type run_and_get_future() = delete;

private:
    void run()
    {
        auto future = run_and_get_future();
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
