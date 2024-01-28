#ifndef KOIOS_GET_HANDLE_AW_H
#define KOIOS_GET_HANDLE_AW_H

#include "koios/macros.h"
#include "koios/task_on_the_fly.h"

KOIOS_NAMESPACE_BEG

class get_handle_aw
{
public:
    constexpr bool await_ready() const noexcept { return false; }
    void await_suspend(task_on_the_fly h) noexcept;
    void* await_resume() const noexcept { return m_result; }

private:
    void* m_result{};
};

KOIOS_NAMESPACE_END

#endif
