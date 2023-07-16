#ifndef KOIOS_EXPECTED_RESULT_H
#define KOIOS_EXPECTED_RESULT_H

#include <concepts>
#include <memory>
#include <utility>

#include "koios/macros.h"
#include "koios/driver_policy.h"

KOIOS_NAMESPACE_BEG

template<typename T, ::std::default_initializable E>
class expected_result
{
private:
    template<typename, ::std::default_initializable, driver_policy_concept, typename> 
    friend class monad_task;

    expected_result() = default;

public:
    using value_type = T;
    using error_type = E;
    using value_pointer = ::std::unique_ptr<value_type>;

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
    value_pointer& value_storage() noexcept { return m_result; }
    value_type get_value() { return ::std::move(*value_storage()); }
    
private:
    value_pointer m_result;
    error_type m_error{};
};

template<typename E>
expected_result<void, E> unexpected(E&& e)
{
    return { ::std::forward<E>(e) };
}

KOIOS_NAMESPACE_END

#endif
