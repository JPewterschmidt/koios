#ifndef KOIOS_INVOCABLE_QUEUE_WRAPPER_H
#define KOIOS_INVOCABLE_QUEUE_WRAPPER_H

#include <memory>
#include <concepts>
#include <functional>
#include <optional>

#include "koios/macros.h"

KOIOS_NAMESPACE_BEG

class invocable_queue_wrapper
{
public:
    using invocable_type = ::std::function<void()>;

public:
#define CAST(p) (reinterpret_cast<Queue*>(p))

    template<::std::default_initializable Queue>
    invocable_queue_wrapper(Queue&& q)
        : m_storage     { new unsigned char[sizeof(Queue)] }, 
          m_dtor        { +[](void* p) noexcept { CAST(p)->~Queue(); } }, 
          m_empty_impl  { +[](void* q) { return CAST(q)->empty(); } },
          m_enqueue_impl{ +[](void* q, invocable_type&& func) { CAST(q)->enqueue(::std::move(func)); } }, 
          m_dequeue_impl{ +[](void* q) { return CAST(q)->dequeue(); } }
    {
        ::std::construct_at(CAST(m_storage.get()), ::std::forward<Queue>(q));
    }

#undef CAST

    ~invocable_queue_wrapper() noexcept;
    invocable_queue_wrapper(invocable_queue_wrapper&& other) noexcept;

    void enqueue(invocable_type&& func) const;
    ::std::optional<invocable_type> dequeue() const;
    bool empty() const;

private:
    ::std::unique_ptr<unsigned char[]> m_storage;
    void (*m_dtor) (void*);
    bool (*const m_empty_impl) (void*);
    void (*const m_enqueue_impl) (void*, invocable_type&&);
    ::std::optional<invocable_type> (*const m_dequeue_impl) (void*);
};

KOIOS_NAMESPACE_END

#endif
