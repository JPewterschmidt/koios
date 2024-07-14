// This file is part of Koios
// https://github.com/JPewterschmidt/koios
//
// Copyleft 2023 - 2024, ShiXin Wang. All wrongs reserved.

#ifndef KOIOS_FUTURE_H
#define KOIOS_FUTURE_H

#include <atomic>
#include <exception>
#include <memory>
#include <type_traits>
#include <optional>
#include <condition_variable> 
#include <mutex>

#include "toolpex/assert.h"

#include "koios/macros.h"
#include "koios/exceptions.h"
#include "koios/future_aw.h"

KOIOS_NAMESPACE_BEG

namespace fp_detials
{
    template<typename>
    class promise_impl;
    
    template<typename>
    class future_impl;

    template<typename> 
    class promise_base;

    template<typename> 
    class future_base;

    class promise_storage_base
    {
    public:
        auto& exception_ptr() { return m_exception; }
        void set_exception_ptr(::std::exception_ptr e) { m_exception = ::std::move(e); }

    private:
        ::std::exception_ptr m_exception;
    };

    template<typename Ret>
    requires (!::std::is_reference_v<Ret>)
    class promise_storage : public promise_storage_base
    {
    public:
        template<typename... Args>
        void construct(Args&&... val)
        {
            new (this->value_buffer_ptr()) Ret(::std::forward<Args>(val)...);
        }

        void destruct() { this->value_buffer_ptr()->~Ret(); }
        Ret& ref() { return *this->value_buffer_ptr(); }

        auto move_out()
        {
            auto result = ::std::move(ref());
            this->destruct();
            return result;
        }

    private:
        auto* value_buffer_ptr() noexcept { return reinterpret_cast<Ret*>(m_val); }

    private:
        unsigned char m_val[sizeof(Ret)]{};
    };

    template<> class promise_storage<void> : public promise_storage_base { };

    template<typename T>
    requires (!::std::is_reference_v<T>)
    struct counterpart_future
    {
    protected:
        ::std::shared_ptr<future_impl<T>> m_future_ptr;   
        auto& f_ptr_ref() noexcept { return m_future_ptr; }
        const auto& f_ptr_ref() const noexcept { return m_future_ptr; }
        auto get_shared_state_lock() const noexcept { return m_future_ptr->get_shared_state_lock(); }
    };

    template<typename T>
    class storage_deliver : protected counterpart_future<T>
    {
    public:
        using value_type = ::std::remove_reference_t<T>;

    protected:
        void send() noexcept { this->f_ptr_ref()->set_storage_ptr(storage()); }
        void send(const ::std::unique_lock<::std::mutex>& lk) noexcept { this->f_ptr_ref()->set_storage_ptr(storage(), lk); }
        auto& storage() { return m_storage; }
        bool future_ready() const noexcept { return this->f_ptr_ref()->ready(); }
        auto get_shared_state_lock() const noexcept { return this->counterpart_future<T>::get_shared_state_lock(); }

    private:
        ::std::shared_ptr<promise_storage<value_type>> m_storage{ 
            ::std::make_shared<promise_storage<value_type>>() 
        };
    };

    template<typename T>
    class promise_impl_base : protected storage_deliver<T>
    {
    public:
        template<typename... Args>
        void set_value(Args&&... val) 
        {
            this->storage()->construct(::std::forward<Args>(val)...);
            this->send();
        }

        template<typename... Args>
        void set_value(const ::std::unique_lock<::std::mutex>& lk, Args&&... val) 
        {
            this->storage()->construct(::std::forward<Args>(val)...);
            this->send(lk);
        }
    };

    template<>
    class promise_impl_base<void> : protected storage_deliver<void>
    { 
    public:
        void set_value() noexcept { this->send(); }
        void set_value(const ::std::unique_lock<::std::mutex>& lk) noexcept { this->send(lk); }
    };
    
    template<typename T>
    class promise_impl 
        : public ::std::enable_shared_from_this<promise_impl<T>>, 
          public promise_impl_base<T>
    {
    public:
        using value_type = T;

    public:
        promise_impl() { this->m_future_ptr.reset(new future_impl<value_type>{}); }

        void weak_link_to_counterpart_future() { }
        promise_impl(promise_impl&& other) noexcept = default;
        promise_impl& operator=(promise_impl&&) noexcept = default;
        auto get_future_impl_p() { return this->f_ptr_ref(); }

        void set_exception(::std::exception_ptr e)
        {
            this->storage()->set_exception_ptr(::std::move(e));
            this->send();
        }

        void set_exception(const ::std::unique_lock<::std::mutex>& lk, ::std::exception_ptr e)
        {
            this->storage()->set_exception_ptr(::std::move(e));
            this->send(lk);
        }

        bool future_ready() const noexcept { return this->promise_impl_base<T>::future_ready(); }
        auto get_shared_state_lock() const noexcept { return this->promise_impl_base<T>::get_shared_state_lock(); }
    };

    template<typename Result>
    class future_impl
    {
    public:
        using value_type = ::std::remove_reference_t<Result>;
        template<typename> friend class fp_detials::promise_impl;
        template<typename> friend class fp_detials::promise_base;
        template<typename> friend class fp_detials::future_base;
        template<typename> friend class fp_detials::storage_deliver;

    public:
        // Operations with locking
        bool ready() const noexcept
        {
            auto lk = this->get_unique_lock();
            return this->ready(lk);
        }

        auto get()
        {
            auto lk = this->get_unique_lock();
            return this->get(lk);
        }

        auto get_nonblk()
        {
            auto lk = this->get_unique_lock();
            if (!this->ready(lk)) [[unlikely]]
                throw koios::future_exception{};
            return this->get_nonblk(lk);
        }

        // Operations needs a lock acquire from `get_shared_state_lock()`
        auto get_shared_state_lock() const noexcept { return this->get_unique_lock(); }

        auto get_nonblk(const ::std::unique_lock<::std::mutex>& lk)
        {
            toolpex_assert(lk.mutex() == &m_lock);
            if (auto& ex = m_storage_ptr->exception_ptr(); ex)
            {
                ::std::rethrow_exception(ex);
            }
            if constexpr (::std::same_as<value_type, void>)
                return;
            else return m_storage_ptr->move_out();
        }

        bool ready(const ::std::unique_lock<::std::mutex>& lk) const noexcept 
        { 
            toolpex_assert(lk.mutex() == &m_lock);
            return !!m_storage_ptr; 
        }

        auto get(::std::unique_lock<::std::mutex>& lk)
        {
            toolpex_assert(lk.mutex() == &m_lock);
            if (!this->ready(lk)) 
                m_cond.wait(lk, [&lk, this]{ return this->ready(lk); });
            return this->get_nonblk(lk);
        }

    private:
        void set_future_aw_ptr(future_aw_detial* ptr)
        {
            auto lk = this->get_unique_lock();
            toolpex_assert(m_aw == nullptr);
            m_aw = ptr;
            if (this->ready(lk)) [[unlikely]]
            {
                m_aw->awake();
            }
        }

        void set_storage_ptr(::std::shared_ptr<promise_storage<value_type>> ptr)
        {
            auto lk = this->get_unique_lock();
            this->set_storage_ptr(::std::move(ptr), lk);
        }

        void set_storage_ptr(::std::shared_ptr<promise_storage<value_type>> ptr, 
                             const ::std::unique_lock<::std::mutex>& lk)
        {
            toolpex_assert(lk.mutex() == &m_lock);
            m_storage_ptr = ::std::move(ptr);
            m_cond.notify_all(); 
            if (m_aw) 
            {
                m_aw->awake();
                m_aw = nullptr;
            }
        }

        auto get_unique_lock() const { return ::std::unique_lock{ m_lock }; }

    private:
        ::std::shared_ptr<fp_detials::promise_storage<value_type>> m_storage_ptr;
        ::std::condition_variable m_cond;
        future_aw_detial* m_aw{};
        mutable ::std::mutex m_lock;
    };

    /*! \brief The base class of future object of koios future/promise pattern 
     *
     *  Because those specializations exist, 
     *  this class provide several common operations 
     *  specilalizations could inherited from.
     */
    template<typename Result>
    class future_base
    {
    public:
        using value_type = ::std::remove_reference_t<Result>;
        template<typename> friend class promise_base;

    public:
        constexpr future_base() = default;

        future_base(future_base&& other) noexcept = default;
        future_base& operator=(future_base&& other) noexcept = default;
        future_base(const future_base&) = delete;
        future_base& operator=(const future_base&) = delete;

        future_base(::std::shared_ptr<fp_detials::future_impl<value_type>> fip)
            : m_impl_ptr{ ::std::move(fip) }
        {
        }

    public:
        /*! \return Wether the corresponding promise has set the return value or not.
         *  You can call this function when the future object was default initialzed
         *  (not be genereted from a promise object.)
         */
        bool ready() const noexcept 
        { 
            if (!this->valid()) return false;
            return m_impl_ptr->ready(); 
        }

        /*! \return the return value the corresponding promise object set.
         *
         *  If the corresponding promise object is not set the return value, 
         *  when this member function be called, it will wait for it.
         *
         *  Or, if the return value type set as `void`, this call will 
         *  wait until the promise call `set_value`.
         *
         *  \attention You can NOT call this function when the future object was default initialzed
         *             (not be genereted from a promise object.)
         *             There's a assertion checking this bad behaviour.
         */
        auto get() 
        { 
            toolpex_assert(valid());
            if constexpr (::std::same_as<value_type, void>) 
                m_impl_ptr->get();
            else return m_impl_ptr->get();
        }

        /*! \return the return value the corresponding promise object set.
         *
         *  If the corresponding promise object is not set the return value, 
         *  when this member function be called, it will wait for it.
         *
         *  You are not suppose to call this function when the promise object is not ready.
         *  or, you will receive a `future_exception`.
         *  You can call the `ready()` function to check wether you can call this safely.
         *
         *  \attention You can NOT call this function when the future object was default initialzed
         *             (not be genereted from a promise object.)
         *             There's a assertion checking this bad behaviour.
         */
        auto get_nonblk()
        {
            toolpex_assert(valid());
            if constexpr (::std::same_as<value_type, void>) 
                m_impl_ptr->get_nonblk();
            else return m_impl_ptr->get_nonblk();
        }

        auto get_shared_state_lock() const noexcept 
        { 
            toolpex_assert(valid());
            return m_impl_ptr->get_shared_state_lock(); 
        }

        auto get_nonblk(const ::std::unique_lock<::std::mutex>& lk)
        {
            toolpex_assert(valid());
            if constexpr (::std::same_as<value_type, void>) 
                m_impl_ptr->get_nonblk(lk);
            else return m_impl_ptr->get_nonblk(lk);
        }

        bool ready(const ::std::unique_lock<::std::mutex>& lk) const noexcept 
        { 
            toolpex_assert(valid());
            return m_impl_ptr->ready(lk);
        }

        auto get(::std::unique_lock<::std::mutex>& lk)
        {
            toolpex_assert(valid());
            if constexpr (::std::same_as<value_type, void>) 
                m_impl_ptr->get(lk);
            else return m_impl_ptr->get(lk);
        }

        bool valid() const noexcept { return m_impl_ptr != nullptr; }

        auto get_async() noexcept
        {
            auto result = future_aw{ *this };
            m_impl_ptr->set_future_aw_ptr(result.get_aw_detial_ptr());
            return result;
        }

    private:
        ::std::shared_ptr<fp_detials::future_impl<value_type>> m_impl_ptr;
    };
} // namespace fp_detials

/*! \brief The future object of koios future/promise pattern */
template<typename T> 
class future : public fp_detials::future_base<T> 
{ 
    template<typename> friend class fp_detials::promise_impl;
    template<typename> friend class fp_detials::promise_base;
public:
    using value_type = T;

    constexpr future() = default;
    future(future&&) noexcept = default;
    future& operator=(future&&) noexcept = default;

private:
    future(::std::shared_ptr<fp_detials::future_impl<value_type>> fip)
        : fp_detials::future_base<value_type>{ ::std::move(fip) }
    {
    }
};

/*! \brief The future object of koios future/promise pattern (reference specilalization) */
template<typename T>
class future<T&> : public fp_detials::future_base<T*>
{
public:
    using value_type = T;
    using reference_type = T&;
    using pointer_type = T*;

    template<typename> friend class fp_detials::promise_impl;
    template<typename> friend class fp_detials::promise_base;

public:
    constexpr future() = default;
    future(future&&) noexcept = default;
    future& operator=(future&&) noexcept = default;
    future(::std::shared_ptr<fp_detials::future_impl<pointer_type>> fip)
        : fp_detials::future_base<pointer_type>{ ::std::move(fip) }
    {
    }

public:
    reference_type get()        { return *this->fp_detials::future_base<pointer_type>::get(); }
    reference_type get_nonblk() { return *this->fp_detials::future_base<pointer_type>::get_nonblk(); }

    reference_type get(const ::std::unique_lock<::std::mutex>& lk)        
    { 
        return *this->fp_detials::future_base<pointer_type>::get(lk); 
    }

    reference_type get_nonblk(::std::unique_lock<::std::mutex>& lk)
    { 
        return *this->fp_detials::future_base<pointer_type>::get_nonblk(lk); 
    }
};

namespace fp_detials
{
    template<typename T>
    class promise_base
    {
    public:
        using value_type = T;

    public:
        promise_base()
            : m_impl_ptr{ ::std::make_shared<fp_detials::promise_impl<value_type>>() } 
        {
            m_impl_ptr->weak_link_to_counterpart_future();
        }

        promise_base(const promise_base&) = delete;
        promise_base& operator=(const promise_base&) = delete;
        promise_base(promise_base&&) noexcept = default;
        promise_base& operator=(promise_base&&) noexcept = default;

    public:
        future<value_type> get_future() { return { m_impl_ptr->get_future_impl_p() }; }
        void set_exception(::std::exception_ptr e) { m_impl_ptr->set_exception(::std::move(e)); }
        void set_exception(const auto& lk, ::std::exception_ptr e) { m_impl_ptr->set_exception(lk, ::std::move(e)); }
        auto& storage() noexcept { return m_impl_ptr->storage(); }
        bool future_ready() const noexcept { return m_impl_ptr->future_ready(); }
        auto get_shared_state_lock() const noexcept { return m_impl_ptr->get_shared_state_lock(); }

    protected:
        ::std::shared_ptr<fp_detials::promise_impl<value_type>> m_impl_ptr;
    };

    template<typename T>
    class promise_base<T&>
    {
    public:
        using value_type = T;
        using pointer_type = T*;
        using reference_type = value_type&;

    public:
        promise_base()
            : m_impl_ptr{ ::std::make_shared<fp_detials::promise_impl<pointer_type>>() } 
        {
            m_impl_ptr->weak_link_to_counterpart_future();
        }

        promise_base(const promise_base&) = delete;
        promise_base& operator=(const promise_base&) = delete;
        promise_base(promise_base&&) noexcept = default;
        promise_base& operator=(promise_base&&) noexcept = default;

    public:
        future<reference_type> get_future() { return { m_impl_ptr->get_future_impl_p() }; }
        void set_exception(::std::exception_ptr e) { m_impl_ptr->set_exception(::std::move(e)); }
        void set_exception(const auto& lk, ::std::exception_ptr e) { m_impl_ptr->set_exception(lk, ::std::move(e)); }
        auto& storage() noexcept { return m_impl_ptr->storage(); }
        bool future_ready() const noexcept { return m_impl_ptr->future_ready(); }
        auto get_shared_state_lock() const noexcept { return m_impl_ptr->get_shared_state_lock(); }

    protected:
        ::std::shared_ptr<fp_detials::promise_impl<pointer_type>> m_impl_ptr;
    };
}

/*! \brief  The promise class of koios future/promise pattern. */
template<typename Result>
class promise : public fp_detials::promise_base<Result>
{
public:
    using value_type = Result;

    /*! \brief Deliver the async task reuslt to corresponding future object.
     *
     *  If the future object was blocked for waiting result value, 
     *  this function call will wake up the clocked future object.
     */
    template<typename T>
    void set_value(T&& val) { this->m_impl_ptr->set_value(::std::forward<T>(val)); }

    template<typename T>
    void set_value(const auto& lk, T&& val) { this->m_impl_ptr->set_value(lk, ::std::forward<T>(val)); }
};

template<typename T>
class promise<T&> : public fp_detials::promise_base<T&>
{
public:
    using value_type = T;
    using reference_type = T&;
    void set_value(reference_type ref) { this->m_impl_ptr->set_value(&ref); }
    void set_value(const auto& lk, reference_type ref) { this->m_impl_ptr->set_value(lk, &ref); }
};

template<>
class promise<void> : public fp_detials::promise_base<void>
{
public:
    using value_type = void;

    /*! \brief Do nothing but tell the future object: I'm ready.
     *  \see `promise`
     */
    void set_value() const noexcept { this->m_impl_ptr->set_value(); }
    void set_value(const auto& lk) const noexcept { this->m_impl_ptr->set_value(lk); }
};

KOIOS_NAMESPACE_END

#endif
