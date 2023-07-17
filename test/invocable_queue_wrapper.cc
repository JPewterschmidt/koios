#include "gtest/gtest.h"
#include "koios/invocable_queue_wrapper.h"

#include <queue>

namespace
{
    int basic_count{};
    int dtor_count{};

    struct queue_wrapper
    {
    public:
        using invocable_type = ::std::function<void()>;
        void enqueue(invocable_type&& func)
        {
            m_q.emplace(::std::move(func));
        }

        ::std::optional<invocable_type> dequeue()
        {
            if (m_q.empty()) return {};
            auto result = m_q.front();
            m_q.pop();
            return { result };
        }

        bool empty() const noexcept { return m_q.empty(); }

        ~queue_wrapper() noexcept
        {
            ++dtor_count;
        }

    private:
        ::std::queue<::std::function<void()>> m_q;   
    };
}

TEST(invocable_queue_wrapper, special_member_func)
{
    koios::invocable_queue_wrapper iqw{ queue_wrapper{} };
    koios::invocable_queue_wrapper another_iqw{ ::std::move(iqw) };
    ASSERT_EQ(dtor_count, 1);
}

TEST(invocable_queue_wrapper, basic)
{
    koios::invocable_queue_wrapper iqw{ queue_wrapper{} };
    for (size_t i{}; i < 10; ++i)
        iqw.enqueue([]{ ++basic_count; });

    for (size_t i{}; i < 10; ++i)
    {
        auto ret = iqw.dequeue();
        if (ret) (*ret)();
    }

    ASSERT_EQ(basic_count, 10);
    ASSERT_EQ(iqw.empty(), true);
}
