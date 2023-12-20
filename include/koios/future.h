#ifndef KOIOS_FUTURE_H
#define KOIOS_FUTURE_H

#include <atomic>
#include <exception>
#include <memory>
#include <cassert>
#include <optional>

#include "koios/macros.h"
#include "koios/exceptions.h"

KOIOS_NAMESPACE_BEG

namespace fp_detials
{
    template<typename>
    class promise_impl;
    
    template<typename>
    class future_impl;

    template<typename Ret>
    struct promise_storage
    {
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

        auto& exception_ptr() { return m_exception; }
        void set_exception_ptr(::std::exception_ptr e)
        {
            m_exception = ::std::move(e);
        }

    private:
        auto* value_buffer_ptr() noexcept 
        {
            return reinterpret_cast<Ret*>(m_val);
        }

    private:
        ::std::exception_ptr m_exception;
        unsigned char m_val[sizeof(Ret)]{};
    };

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
        void send() 
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
        }
    };

    template<>
    class promise_impl_base<void> : protected storage_deliver<void>
    { 
    public:
        constexpr void set_value() const noexcept;
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
            this->m_future_ptr = ::std::make_shared<future_impl<value_type>>();
        }

        void weak_link_to_counterpart_future()
        {
            this->f_ptr_ref()->set_counterpart_promise_ptr(this->shared_from_this());
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
        }

        void send() { this->storage_deliver<value_type>::send(); }
    };

    template<typename Result>
    class future_impl
    {
    public:
        using value_type = Result;
        template<typename> friend class promise_impl;
        template<typename> friend class storage_deliver;

    public:
        bool ready() const noexcept
        {
            return m_storage_ptr.load() != nullptr;
        }

        value_type get()
        {
            auto s = m_storage_ptr.load();
            assert(s);
            if (auto& ex = s->exception_ptr(); ex)
            {
                ::std::rethrow_exception(ex);
            }
            return s->move_out();
        }

        bool valid() const noexcept
        {
            auto s = m_storage_ptr.load();
            return s || !m_promise_wptr.expired();
        }

    private:
        void set_counterpart_promise_ptr(::std::weak_ptr<promise_impl<value_type>> wpl)
        {
            m_promise_wptr = ::std::move(wpl);
        }

        void set_storage_ptr(::std::shared_ptr<promise_storage<value_type>> ptr)
        {
            m_storage_ptr.store(::std::move(ptr));
        }

    private:
        ::std::atomic<::std::shared_ptr<fp_detials::promise_storage<value_type>>> m_storage_ptr;
        ::std::weak_ptr<promise_impl<value_type>> m_promise_wptr;
    };
}

template<typename Result>
class future
{
public:
    using value_type = Result;
    template<typename> friend class promise_base;

public:
    constexpr future() = default;
    future(future&& other) noexcept = default;
    future& operator=(future&& other) noexcept = default;

private:
    future(::std::shared_ptr<fp_detials::future_impl<value_type>> fip)
        : m_impl_ptr{ ::std::move(fip) }
    {
    }

public:
    bool ready() const noexcept 
    { 
        if (!m_impl_ptr) return false;
        return m_impl_ptr->ready(); 
    }

    value_type get() 
    { 
        if (!ready()) throw future_exception{};
        return { m_impl_ptr->get() };
    }

    bool valid() const noexcept 
    { 
        if (!m_impl_ptr) return false;
        return m_impl_ptr->valid(); 
    }

private:
    ::std::shared_ptr<fp_detials::future_impl<value_type>> m_impl_ptr;
};

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

    future<value_type> get_future()
    {
        return { m_impl_ptr->get_future_impl_p() };
    }

    void set_exception(::std::exception_ptr e)
    {
        m_impl_ptr->set_exception(::std::move(e));
    }

    void send() noexcept { m_impl_ptr->send(); }

    auto& storage() noexcept { return m_impl_ptr->storage(); }

protected:
    ::std::shared_ptr<fp_detials::promise_impl<value_type>> m_impl_ptr;
};

template<typename Result>
class promise : public promise_base<Result>
{
public:
    using value_type = Result;

    template<typename T>
    void set_value(T&& val)
    {
        this->m_impl_ptr->set_value(::std::forward<T>(val));
    }
};

template<>
class promise<void> : public promise_base<void>
{
public:
    using value_type = void;
    constexpr void set_value() const noexcept {}
};

KOIOS_NAMESPACE_END

#endif
