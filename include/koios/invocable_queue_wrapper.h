#ifndef KOIOS_INVOCABLE_QUEUE_WRAPPER_H
#define KOIOS_INVOCABLE_QUEUE_WRAPPER_H

#include <memory>
#include <concepts>
#include <functional>
#include <optional>

#include "koios/macros.h"

KOIOS_NAMESPACE_BEG

/*! \brief Allow user to use different qeuue implement in `thread_pool` */
class invocable_queue_wrapper
{
public:
    using invocable_type = ::std::move_only_function<void()>;

public:
#define CAST(p) (reinterpret_cast<Queue*>(p))

    /*! \brief All information about the specific queue implementation is confined to the constructor. 
     *         Take ownership of the queue.
     *
     *  This is a typical type erasure class, and the user needs to hand over 
     *  the ownership of the queue to this class. 
     *  It expresses the functionality of interest to `thread_pool` 
     *  as function pointers independent of the template parameters.
     *
     *  \warning This type-erasure class does not provide thread-safe guarantee.
     *  \tparam Queue A arbitrary queue type.
     */
    template<::std::default_initializable Queue>
    invocable_queue_wrapper(Queue&& q)
        : m_storage     { new unsigned char[sizeof(Queue)] }, 
          m_dtor        { +[](void* p) noexcept { CAST(p)->~Queue(); } }, 
          m_empty_impl  { +[](void* q) { return CAST(q)->empty(); } },
          m_enqueue_impl{ +[](void* q, invocable_type&& func) { CAST(q)->enqueue(::std::move(func)); } }, 
          m_dequeue_impl{ +[](void* q) { return CAST(q)->dequeue(); } }, 
          m_size_impl   { +[](void* q) { return CAST(q)->size(); } }
    {
        ::std::construct_at(CAST(m_storage.get()), ::std::forward<Queue>(q));
    }

#undef CAST

    ~invocable_queue_wrapper() noexcept;
    invocable_queue_wrapper(invocable_queue_wrapper&& other) noexcept;

    void enqueue(invocable_type&& func) const;
    ::std::optional<invocable_type> dequeue() const;
    bool empty() const;
    size_t size() const noexcept { return m_size_impl(m_storage.get()); }

private:
    ::std::unique_ptr<unsigned char[]> m_storage;
    void (*m_dtor) (void*);
    bool (*const m_empty_impl) (void*);
    void (*const m_enqueue_impl) (void*, invocable_type&&);
    ::std::optional<invocable_type> (*const m_dequeue_impl) (void*);
    size_t (*const m_size_impl) (void*);
};

KOIOS_NAMESPACE_END

#endif
