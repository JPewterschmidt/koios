#ifndef KOIOS_LITE_FUTURE_H
#define KOIOS_LITE_FUTURE_H

#include "toolpex/callback_promise.h"

#include "koios/task_on_the_fly.h"

namespace koios
{

template<typename T>
struct frame_agg
{
    enum status_t
    {
        NONE, 
        READY, 
        EXCEPT,
    };

    status_t m_status;
    toolpex::future_frame<T> m_t_frame{};
    task_on_the_fly m_waitting{};
};

template<typename T>
class lite_promise
{
public:
    using value_type = T;

public:
    lite_promise(frame_agg<T>* f)
        : m_frame_ptr{ f }
    {
    }

    template<typename... Args>
    void set_value(Args&&... args)
    {
        toolpex_assert(!!m_frame_ptr);
        m_frame_ptr->m_t_frame.set_value(::std::forward<Args>(args)...);
        m_frame_ptr->m_status = frame_agg<T>::READY;
    }

    void set_exception(::std::exception_ptr ex) noexcept
    {
        toolpex_assert(!!m_frame_ptr);
        m_frame_ptr->m_t_frame.set_exception(::std::move(ex));
        m_frame_ptr->m_status = frame_agg<T>::EXCEPT;
    }

private:
    frame_agg<T>* m_frame_ptr{};
};

template<typename T>
class lite_future
{
public:
    using value_type = T;

public:
    lite_promise<T> get_promise()
    {
        return { &m_frame };
    }

    bool ready() const noexcept
    {
        return m_frame.m_status != frame_agg<T>::NONE;
    }

    constexpr bool valid() const noexcept { return true; }

    decltype(auto) get()
    {
        auto& mtframe = m_frame.m_t_frame;
        if (m_frame.m_status == frame_agg<T>::EXCEPT)
        {
            ::std::rethrow_exception(mtframe.get_exception());
        }

        if constexpr (!::std::same_as<T, void>)
        {
            if constexpr (::std::is_reference_v<T>)
            {
                return mtframe.value();
            }
            else
            {
                return ::std::move(mtframe.value());
            }
        }
    }

    decltype(auto) get_nonblk() { return get(); }

    void set_waiting(task_on_the_fly f)
    {
        m_frame.m_waitting = ::std::move(f);
    }
    
private:
    template<typename>
    friend class lite_promise;
    
    frame_agg<T> m_frame;
};

} // namespace koios

#endif
