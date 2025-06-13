#ifndef KOIOS_LITE_FUTURE_H
#define KOIOS_LITE_FUTURE_H

#include "toolpex/callback_promise.h"

#include "koios/waiting_handle.h"
#include "koios/task_on_the_fly.h"
#include "koios/future_aw.h"

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

    status_t m_status{};
    toolpex::future_frame<T> m_t_frame{};
    task_on_the_fly m_waitting{};
};

template<typename T>
class lite_future
{
public:
    using value_type = T;

public:
    constexpr lite_future() noexcept = default;
    lite_future(const lite_future&) = delete;
    lite_future(lite_future&& other) noexcept = default;
    lite_future& operator=(lite_future&& other) noexcept = default;

    lite_future(::std::unique_ptr<frame_agg<T>> fagg)
        : m_frame{ ::std::move(fagg) }
    {
        toolpex_assert(m_frame);
    }

    bool ready() const noexcept
    {
        return m_frame->m_status != frame_agg<T>::NONE;
    }

    constexpr bool valid() const noexcept { return true; }

    decltype(auto) get()
    {
        auto& mtframe = m_frame->m_t_frame;
        if (m_frame->m_status == frame_agg<T>::EXCEPT)
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
        m_frame->m_waitting = ::std::move(f);
    }

    future_aw<lite_future<T>> get_async()
    {
        return { *this };
    }

    future_aw<lite_future<T>> operator co_await ()
    {
        return get_async();
    }
    
private:
    template<typename>
    friend class lite_promise;
    
    ::std::unique_ptr<frame_agg<T>> m_frame{};
};

template<typename T>
class lite_promise
{
public:
    using value_type = T;

public:
    lite_promise() 
        : m_frame_store{ ::std::make_unique<frame_agg<T>>() }, 
          m_frame{ m_frame_store.get() }
    {
    }

    template<typename... Args>
    void set_value(Args&&... args)
    {
        m_frame->m_t_frame.set_value(::std::forward<Args>(args)...);
        m_frame->m_status = frame_agg<T>::READY;
        wake_up(::std::move(m_frame->m_waitting));
    }

    void set_exception(::std::exception_ptr ex) noexcept
    {
        m_frame->m_t_frame.set_exception(::std::move(ex));
        m_frame->m_status = frame_agg<T>::EXCEPT;
        wake_up(::std::move(m_frame->m_waitting));
    }

    lite_future<T> get_future()
    {
        return { ::std::move(m_frame_store) };
    }

private:
    ::std::unique_ptr<frame_agg<T>> m_frame_store{};
    frame_agg<T>* m_frame{};
};

} // namespace koios

#endif
