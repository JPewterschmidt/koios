#ifndef KOIOS_FUTURE_H
#define KOIOS_FUTURE_H

#include <atomic>
#include <exception>
#include <memory>
#include <cassert>
#include <optional>
#include <condition_variable> 
#include <mutex>

#include "koios/macros.h"
#include "koios/exceptions.h"

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
        void set_exception_ptr(::std::exception_ptr e)
        {
            m_exception = ::std::move(e);
        }

    private:
        ::std::exception_ptr m_exception;
    };

    template<typename Ret>
    class promise_storage : public promise_storage_base
    {
    public:
        template<typename T>
        void construct(T&& val)
        {
            new (this->value_buffer_ptr()) Ret(::std::forward<T>(val));
        }

        void destruct()
        {
            this->value_buffer_ptr()->~Ret();
        }

        Ret& ref()
        {
            return *value_buffer_ptr();
        }

        auto move_out()
        {
            auto result = ::std::move(ref());
            destruct();
            return result;
        }

    private:
        auto* value_buffer_ptr() noexcept 
        {
            return reinterpret_cast<Ret*>(m_val);
        }

    private:
        unsigned char m_val[sizeof(Ret)]{};
    };

    template<> class promise_storage<void> : public promise_storage_base { };

    template<typename T>
    struct counterpart_future
    {
    protected:
        ::std::shared_ptr<future_impl<T>> m_future_ptr;   
        auto& f_ptr_ref() noexcept { return m_future_ptr; }
    };

    template<typename T>
    class storage_deliver : protected counterpart_future<T>
    {
    public:
        using value_type = T;

    protected:
        void send() noexcept
        {
            this->f_ptr_ref()->set_storage_ptr(storage());
        }

        auto& storage() { return m_storage; }

    private:
        ::std::shared_ptr<promise_storage<value_type>> m_storage{ 
            ::std::make_shared<promise_storage<value_type>>() 
        };
    };

    template<typename T>
    class promise_impl_base : protected storage_deliver<T>
    {
    public:
        template<typename TT>
        void set_value(TT&& val) 
        {
            this->storage()->construct(::std::forward<T>(val));
            this->send();
        }
    };

    template<>
    class promise_impl_base<void> : protected storage_deliver<void>
    { 
    public:
        void set_value() noexcept
        {
            this->send();
        }
    };
    
    template<typename T>
    class promise_impl 
        : public ::std::enable_shared_from_this<promise_impl<T>>, 
          public promise_impl_base<T>
    {
    public:
        using value_type = T;

    public:
        promise_impl()
        {
            //this->m_future_ptr = ::std::make_shared<future_impl<value_type>>();
            this->m_future_ptr.reset(new future_impl<value_type>{});
        }

        void weak_link_to_counterpart_future()
        {
        }

        promise_impl(promise_impl&& other) noexcept = default;
        promise_impl& operator=(promise_impl&&) noexcept = default;

        auto get_future_impl_p()
        {
            return this->f_ptr_ref();
        }

        void set_exception(::std::exception_ptr e)
        {
            this->storage()->set_exception_ptr(::std::move(e));
            this->send();
        }
    };

    template<typename Result>
    class future_impl
    {
    public:
        using value_type = Result;
        template<typename> friend class fp_detials::promise_impl;
        template<typename> friend class fp_detials::promise_base;
        template<typename> friend class fp_detials::storage_deliver;

    public:
        bool ready() const noexcept
        {
            ::std::lock_guard lk{ m_lock };
            return ready_nolock();
        }

        auto get()
        {
            auto lk = get_unique_lock();
            if (!m_storage_ptr) 
                m_cond.wait(lk, [this]{ return ready_nolock(); });

            return get_nonblk(::std::move(lk));
        }

        auto get_nonblk()
        {
            if (!m_storage_ptr) [[unlikely]]
                throw koios::future_exception{};
            return get_nonblk(get_unique_lock());
        }

    private:
        void set_storage_ptr(::std::shared_ptr<promise_storage<value_type>> ptr)
        {
            ::std::lock_guard lk{ m_lock };
            m_storage_ptr = ::std::move(ptr);
            m_cond.notify_all(); 
        }

        bool ready_nolock() const noexcept
        {
            return !!m_storage_ptr;
        }

        auto get_unique_lock()
        {
            return ::std::unique_lock{ m_lock };
        }

        auto get_nonblk(::std::unique_lock<::std::mutex> lk)
        {
            if (auto& ex = m_storage_ptr->exception_ptr(); ex)
            {
                ::std::rethrow_exception(ex);
            }
            if constexpr (::std::same_as<value_type, void>)
                return;
            else return m_storage_ptr->move_out();
        }

    private:
        ::std::shared_ptr<fp_detials::promise_storage<value_type>> m_storage_ptr;
        ::std::condition_variable m_cond;
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
        using value_type = Result;
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
            if (!m_impl_ptr) return false;
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
            assert(m_impl_ptr);
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
            assert(m_impl_ptr);
            if constexpr (::std::same_as<value_type, void>) 
                m_impl_ptr->get_nonblk();
            else return m_impl_ptr->get_nonblk();
        }

    private:
        ::std::shared_ptr<fp_detials::future_impl<value_type>> m_impl_ptr;
    };
}

/*! \brief The future object of koios future/promise pattern */
template<typename T> 
class future : public fp_detials::future_base<T> 
{ 
    template<typename> friend class fp_detials::promise_impl;
    template<typename> friend class fp_detials::promise_base;
public:
    using value_type = T;

    constexpr future() = default;

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
    future(::std::shared_ptr<fp_detials::future_impl<pointer_type>> fip)
        : fp_detials::future_base<pointer_type>{ ::std::move(fip) }
    {
    }

public:
    reference_type get() 
    { 
        return *fp_detials::future_base<pointer_type>::get();
    }

    reference_type get_nonblk()
    {
        return *fp_detials::future_base<pointer_type>::get_nonblk();
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
        future<value_type> get_future()
        {
            return { m_impl_ptr->get_future_impl_p() };
        }

        void set_exception(::std::exception_ptr e)
        {
            m_impl_ptr->set_exception(::std::move(e));
        }

        auto& storage() noexcept { return m_impl_ptr->storage(); }

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
        future<reference_type> get_future()
        {
            return { m_impl_ptr->get_future_impl_p() };
        }

        void set_exception(::std::exception_ptr e)
        {
            m_impl_ptr->set_exception(::std::move(e));
        }

        auto& storage() noexcept { return m_impl_ptr->storage(); }

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
    void set_value(T&& val)
    {
        this->m_impl_ptr->set_value(::std::forward<T>(val));
    }
};

template<typename T>
class promise<T&> : public fp_detials::promise_base<T&>
{
public:
    using value_type = T;
    using reference_type = T&;
    
    void set_value(reference_type ref)
    {
        this->m_impl_ptr->set_value(&ref);
    }
};

template<>
class promise<void> : public fp_detials::promise_base<void>
{
public:
    using value_type = void;

    /*! \brief Do nothing but tell the future object: I'm ready.
     *  \see `promise`
     */
    void set_value() const noexcept 
    {
        this->m_impl_ptr->set_value();
    }
};

KOIOS_NAMESPACE_END

#endif
