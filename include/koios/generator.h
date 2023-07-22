#ifndef KOIOS_GENERATOR_H
#define KOIOS_GENERATOR_H

#include <coroutine>
#include <memory>
#include <cassert>
#include <utility>
#include <stdexcept>
#include <memory>

#include "koios/macros.h"
#include "koios/promise_base.h"
#include "koios/driver_policy.h"
#include "koios/generator_iterator.h"

KOIOS_NAMESPACE_BEG

template<typename T, typename Alloc>
struct _generator
{
    struct [[nodiscard]] _type;
};

/*! \brief The `generator` specific promise class.
 *  The ownership of handler won't move out.
 *
 *  \tparam Alloc The allocator type used.
 */
template<typename T, typename Alloc>
class generator_promise_type : public promise_base<::std::suspend_always>
{
private:
    /*! \brief allocate memory then initialize the object at.
     *  \return the pointer to the memory buffer allocated.
     */
    template<typename TT>
    static T* alloc_and_construct(TT&& tt)
    {
        T* buffer = Alloc{}.allocate(sizeof(T));
        ::std::construct_at(buffer, ::std::forward<TT>(tt));
        return buffer;
    }

    /*! The deleter for `::std::unique_ptr`, which firstly destruct the object on it,
     *  the deallocate it by the allocator.
     */
    struct value_deleter
    {
        void operator()(T* p) const noexcept 
        {
            p->~T();
            Alloc{}.deallocate(p, sizeof(*p));
        }
    };

public:
    using handle_type = ::std::coroutine_handle<generator_promise_type<T, Alloc>>;
    using storage_type = ::std::unique_ptr<T, value_deleter>;

    storage_type m_current_value_p{ nullptr }; /*! Holds the memory and the return value object. */

    static _generator<T, Alloc>::_type get_return_object_on_allocation_failure() 
    { 
        return { handle_type{} }; 
    }

    _generator<T, Alloc>::_type get_return_object() 
    { 
        return { handle_type::from_promise(*this) }; 
    }

    constexpr void return_void() const noexcept {}

    void unhandled_exception() const { throw; }

    /*! \brief Function which stores the yield value.
     *  After store the yield value, this will makes the generator coroutine suspend and back to the caller function.
     */
    template<typename TT>
    auto yield_value(TT&& val)
    {
        m_current_value_p.reset(
            alloc_and_construct(::std::forward<TT>(val))
        );
        return ::std::suspend_always{};
    }

    /*! \retval true Current yield value was not been moved.
     *  \retval false Current yield value was moved out.
     */
    bool has_value() const noexcept
    {
        return bool(m_current_value_p);
    }

    /*! \brief Take the ownership of the current yield value.
     *  \return The current yield value object.
     */
    T value()
    {
        auto resultp = ::std::move(m_current_value_p);
        return ::std::move(*resultp);
    }

    /*! \return Reference of the storage which holds the yield value and its memory buffer. */
    auto& value_storage() noexcept { return m_current_value_p; }

    /*! \brief destruct the current yield value and deallocate the memory. */
    void clear() noexcept { m_current_value_p.reset(); }
};

/*! \brief The generator type
 *  \tparam T the yield value type.
 *  \tparam Alloc the allocator type.
 *
 *  This type will always holds this ownership of the coroutine handler.
 */
template<typename T, typename Alloc>
class _generator<T, Alloc>::_type
{
    static_assert(!::std::is_reference_v<T>, "The `result_type` of generator could not be a reference!");
public:
    friend class generator_promise_type<T, Alloc>;
    using promise_type = generator_promise_type<T, Alloc>;
    using result_type = T;
    using allocator = Alloc;

    /*! \brief Resume the execution of the generator, and update the yield value.
     *  \retval true Move successfully, you could take the newly yield value.
     *  \retval false Move failure, usually caused by there're actuall no more yield value.
     */
    bool move_next()
    {
        if (m_coro == nullptr)
            return false;
        m_coro.promise().clear();
        return (m_coro.resume(), !m_coro.done());
    }

    /*! \retval true There's a yield value you could retrive.
     *  \retval false There's no any yield value you could retrived.
     */
    bool has_value() const noexcept { return m_coro ? m_coro.promise().has_value() : false; }

    /*! \return the non-constant reference type of the yield value. 
     *  You could call this with `std::move`
     */
    decltype(auto) current_value()
    {
        auto& promise = m_coro.promise();
        if (!promise.has_value()) 
        {
            throw ::std::out_of_range{ 
                "There's no next val could be retrived, call `has_value()` first! "
                "Or, you may perform co_await call, it's not supported yet."
            };
        }
        return promise.value();
    }

    /*! \return The reference of the storage which holds the memory buffer and the yield value.
     */
    auto& current_value_storage()
    {
        return m_coro.promise().value_storage();
    }

    _type(const _type&) = delete;
    _type(_type&& other) noexcept
        : m_coro{ ::std::exchange(other.m_coro, nullptr) }, 
          m_need_destroy_in_dtor{ ::std::exchange(other.m_need_destroy_in_dtor, false) }
    {
    }

    _type& operator = (_type&& other) noexcept
    {
        destroy_current_coro();

        m_coro = ::std::exchange(other.m_coro, nullptr);
        m_need_destroy_in_dtor = ::std::exchange(other.m_need_destroy_in_dtor, false);

        return *this;
    }

    /*! \brief Will destroy the coroutine handler, wether the generator has been `move_next`
     */
    ~_type() noexcept { destroy_current_coro(); }

private:
    _type(::std::coroutine_handle<promise_type> h) 
        : m_coro{ h } 
    {
    }

    void destroy_current_coro() noexcept
    {
        if (::std::exchange(m_need_destroy_in_dtor, false)) [[likely]]
            m_coro.destroy();
    }

    ::std::coroutine_handle<promise_type> m_coro;
    bool m_need_destroy_in_dtor{ true };

public:
    using iterator = detial::generator_iterator<_generator<T, Alloc>::_type>;
    iterator begin() noexcept { return { *this }; }
    constexpr detial::generator_iterator_sentinel end() const noexcept { return {}; };
};


template<typename T, typename Alloc = ::std::allocator<T>>
using generator = typename _generator<T, Alloc>::_type;

KOIOS_NAMESPACE_END

#endif
