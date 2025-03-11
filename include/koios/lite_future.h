#ifndef KOIOS_LITE_FUTURE_H
#define KOIOS_LITE_FUTURE_H

#include "toolpex/callback_promise.h"

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
};

template<typename T>
class lite_promise
{
public:
    lite_promise(frame_agg<T>* f)
        : m_frame_ptr{ f }
    {
    }

    template<typename... Args>
    void set_value(Args&&... args)
    {
        toolpex_assert(!!m_frame_ptr);
        auto& [status, frame_p] = *m_frame_ptr;
        frame_p.set_value(::std::forward<Args>(args)...);
        status = frame_agg<T>::READY;
    }

    void set_exception(::std::exception_ptr ex) noexcept
    {
        toolpex_assert(!!m_frame_ptr);
        auto& [status, frame_p] = *m_frame_ptr;
        frame_p.set_exception(::std::move(ex));
        status = frame_agg<T>::EXCEPT;
    }

private:
    frame_agg<T>* m_frame_ptr{};
};

template<typename T>
class lite_future
{
public:
    lite_promise<T> get_promise()
    {
        return { &m_frame };
    }

    bool ready() const noexcept
    {
        return m_frame.m_status != frame_agg<T>::NONE;
    }

    decltype(auto) get()
    {
        auto& [status, mtframe] = m_frame;
        if (status == frame_agg<T>::EXCEPT)
        {
            ::std::rethrow_exception(mtframe.get_exception());
        }

        if constexpr (::std::is_reference_v<T>)
        {
            return mtframe.value();
        }
        else
        {
            return ::std::move(mtframe.value());
        }
    }
    
private:
    template<typename>
    friend class lite_promise;
    
    frame_agg<T> m_frame;
};

} // namespace koios

#endif
