#ifndef KOIOS_GENERATOR_ITERATOR_H
#define KOIOS_GENERATOR_ITERATOR_H

#include <memory>
#include <cstddef>
#include <iterator>

#include "koios/macros.h"
#include "koios/generator_concepts.h"

KOIOS_NAMESPACE_BEG

namespace detial
{
    class generator_iterator_sentinel{};

    //template<generator_concept G>
    template<typename G>
    class generator_iterator
    {
    private:
        using generator_type = G;
        using result_type = typename generator_type::result_type;

        using difference_type = ::std::size_t;
        using value_type = result_type;
        using pointer = value_type*;
        using reference = value_type&;
        using iterator_category = ::std::forward_iterator_tag;

    public:
        generator_iterator(generator_type& g) noexcept
            : m_generator{ g }
        {
            m_reach_end = !m_generator.move_next();
        }

        generator_iterator(const generator_iterator&) = delete;
        generator_iterator& operator=(const generator_iterator&) = delete;

        generator_iterator& operator++()
        {
            m_storage = nullptr;
            m_reach_end = !m_generator.move_next() || !m_generator.has_value();

            return *this;
        }

        void operator++(int) { operator++(); }

        generator_iterator& operator=(generator_iterator_sentinel)
        {
            m_storage = nullptr;
            m_reach_end = true;
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
                m_storage.swap(m_generator.current_value_storage());
                return bool(m_storage);
            }
            return true;
        }

        template<generator_concept GG>
        friend bool operator != (const generator_iterator<GG>&, const generator_iterator_sentinel&);

    private: 
        generator_type& m_generator;
        bool m_reach_end{};
        ::std::unique_ptr<result_type> m_storage;
    };

    template<generator_concept G>
    bool operator!=(const generator_iterator<G>& iter, const generator_iterator_sentinel&)
    {
        return !iter.m_reach_end;
    }
}

KOIOS_NAMESPACE_END

#endif
