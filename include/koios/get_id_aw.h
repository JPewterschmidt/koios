#ifndef KOIOS_GET_HANDLE_AW_H
#define KOIOS_GET_HANDLE_AW_H

#include "koios/macros.h"
#include "koios/task_on_the_fly.h"
#include <cstddef>
#include <cstdint>
#include <functional>

KOIOS_NAMESPACE_BEG

using task_id = void*;

class get_id_aw
{
public:
    constexpr bool await_ready() const noexcept { return false; }
    void await_suspend(task_on_the_fly h) noexcept;
    task_id await_resume() const noexcept { return m_result; }

private:
    task_id m_result{};
};

KOIOS_NAMESPACE_END

#endif
