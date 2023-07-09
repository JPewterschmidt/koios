#ifndef KOIOS_GENERATOR_ITERATOR_H
#define KOIOS_GENERATOR_ITERATOR_H

#include <memory>

#include "koios/macros.h"
#include "koios/generator_concepts.h"

KOIOS_NAMESPACE_BEG

namespace detial
{
    class generator_iterator_sentinel{};

    template<value_result_generator_concept G>
    class generator_iterator
    {
    private:
        using generator_type = G;
        using result_type = typename generator_type::result_type;

    public:
        generator_iterator(generator_type g) noexcept
            : m_generator{ g }
        {
        }

        generator_iterator(const generator_iterator&) = delete;
        generator_iterator& operator=(const generator_iterator&) = delete;

        generator_iterator& operator++()
        {
            m_storage = nullptr;
            if (!m_generator.move_next())
                m_reach_end = true;
            m_reach_end = m_generator.has_value();

            return *this;
        }

        result_type& operator *()
        {
            if (!retrive_data())
                throw ::std::out_of_range{ "There're no any data could be used!" };
            return *m_storage;
        }

        result_type* operator ->()
        {
            if (!retrive_data())
                throw ::std::out_of_range{ "There're no any data could be used!" };
            return m_storage.get();
        }

    private:
        bool retrive_data()
        {
            if (!m_storage)
            {
                if (!m_generator.has_value())
                    return false;
                m_storage = ::std::make_unique<result_type>(m_generator.value());
            }
            return true;
        }

        template<generator_concept GG>
        friend bool operator == (generator_iterator<GG>, generator_iterator_sentinel);

    private: 
        generator_type& m_generator;
        bool m_reach_end{};
        ::std::unique_ptr<result_type> m_storage;
    };

    template<generator_concept G>
    bool operator==(generator_iterator<G> iter, generator_iterator_sentinel)
    {
        return iter.m_reach_end;
    }
}

KOIOS_NAMESPACE_END

#endif
