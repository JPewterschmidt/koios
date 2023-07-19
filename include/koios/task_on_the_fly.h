#ifndef KOIOS_TASK_ON_THE_FLY_H
#define KOIOS_TASK_ON_THE_FLY_H

#include <coroutine>
#include <utility>

#include "macros.h"

KOIOS_NAMESPACE_BEG

/*! \brief One of the `std::coroutine_handle` ownership phase manager.
 *
 *  When a task was scheduled in `task_scheduler`. 
 *  The ownership of the related `::std::coroutine_handle` will be taken by related `task_scheduler`, 
 *  be hold by this type of object immediately.
 *  When `quick_stop()` of `thread_pool` was called, there might be some task remain in the queue without executing.
 *  Meanwhile this type of objects are still hold the ownership of the `::std::coroutine_handle`.
 *  Then the destructor will ensure to call `destroy()`.
 *
 *  \see `destroy_aw`
 *  \see `task`
 */
class task_on_the_fly
{
public:
    task_on_the_fly(::std::coroutine_handle<> h) 
        : m_h{ h }
    {
    }

    task_on_the_fly(task_on_the_fly&& other) noexcept
        : m_h{ other.m_h }, m_need_destroy{ ::std::exchange(other.m_need_destroy, false) }
    {
    }

    /*! \brief Execute and give up the ownership of the related `std::coroutine_handle`.
     *
     *  This member function will firstly release the ownership of the handler, 
     *  secondly call the `std::coroutine_handle::resume()` of the handler, 
     */
    void operator()()
    { 
        m_need_destroy = false;
        m_h.resume(); 
    }

    /*! If the `operator()()` not called, in other words, 
     *  this `task_on_the_fly` still holds the ownership of the handler, 
     *  the destructor will call `::std::coroutine_handle::destroy()`.
     */
    ~task_on_the_fly() noexcept
    {
        if (m_need_destroy) [[unlikely]] m_h.destroy();
    }

private:
    ::std::coroutine_handle<> m_h;
    bool m_need_destroy{ true };
};

KOIOS_NAMESPACE_END

#endif
