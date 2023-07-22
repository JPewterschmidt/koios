#ifndef KOIOS_TASK_SCHEDULER_WRAPPER_H
#define KOIOS_TASK_SCHEDULER_WRAPPER_H

#include <utility>
#include <type_traits>

#include "koios/macros.h"
#include "koios/task_on_the_fly.h"

KOIOS_NAMESPACE_BEG

/*! \brief A `task_scheduler` type-erasure wrapper.
 *  \warning Which NOT holds the ownership of the `task_scheduler`.
 *           So might be NOT suitable for `local_thread_scheduler`.
 *  \see `task_scheduler_owned_wrapper`
 */
class task_scheduler_wrapper
{
public:
    using is_wrapper_t = ::std::true_type;

public:
    /*! \param scheduler The `task_scheduler` you want wrap.
     *  This woun't take the ownership of the `task_scheduler`.
     */
    template<typename Schr>
    task_scheduler_wrapper(Schr& scheduler) noexcept
        : m_enqueue_impl { 
            [](void* schr, task_on_the_fly h) mutable { 
                static_cast<Schr*>(schr)->enqueue(::std::move(h)); 
            } 
          }, 
          m_schr{ &scheduler }
    {
    }

    task_scheduler_wrapper(const task_scheduler_wrapper&) = delete;
    task_scheduler_wrapper& operator=(const task_scheduler_wrapper&) = delete;

    /*! \brief Call the underlying `task_scheduler::enqueue()`
     *  The behavior based on the referd instance.
     */
    void enqueue(task_on_the_fly h) 
    {
        m_enqueue_impl(m_schr, ::std::move(h));
    }

private:
    void (*m_enqueue_impl)(void*, task_on_the_fly);
    void* const m_schr{};
};

/*! \brief A `task_scheduler` type-erasure wrapper.
 *  \warning Which DO holds the ownership of the `task_scheduler`.
 *           Suitable for `local_thread_scheduler`.
 *  \see `task_scheduler_wrapper`
 */
class task_scheduler_owned_wrapper
{
public:
    using is_wrapper_t = ::std::true_type;

public:
    /*! \brief Allocate enough memory then construct the scheduler on it. */
    template<typename Schr>
    task_scheduler_owned_wrapper(Schr&& scheduler) noexcept
        : m_enqueue_impl { 
            [](void* schr, task_on_the_fly h) mutable { 
                static_cast<Schr*>(schr)->enqueue(::std::move(h)); 
            } 
          }, 
          m_storage{ new unsigned char[sizeof(Schr)] }, 
          m_dtor{ [](void* schr){ static_cast<Schr*>(schr)->~Schr(); } }
    {
        new(m_storage.get()) Schr(::std::forward<Schr>(scheduler));
    }

    ~task_scheduler_owned_wrapper() noexcept
    {
        m_dtor(m_storage.get());
    }

    task_scheduler_owned_wrapper(task_scheduler_owned_wrapper&&) noexcept = default;
    task_scheduler_owned_wrapper(const task_scheduler_owned_wrapper&) = delete;

    /*! \brief Call the underlying `task_scheduler::enqueue()`
     *  The behavior based on the referd instance.
     *  \see `local_thread_scheduler`.
     */
    void enqueue(task_on_the_fly h) 
    {
        m_enqueue_impl(m_storage.get(), ::std::move(h));
    }

private:
    void (*m_enqueue_impl)(void*, task_on_the_fly);
    ::std::unique_ptr<unsigned char[]> m_storage{};
    void (*m_dtor)(void*);
};


KOIOS_NAMESPACE_END

#endif
