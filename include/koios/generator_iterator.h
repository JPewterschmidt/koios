/* Koios, A c++ async runtime library.
 * Copyright (C) 2024  Jeremy Pewterschmidt
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

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
    class sync_generator_iterator_sentinel{};

    template<typename G>
    class sync_generator_iterator
    {
    private:
        using generator_type = G;
        using result_type = typename generator_type::result_type;

        using difference_type = ::std::ptrdiff_t;
        using value_type = result_type;
        using pointer = value_type*;
        using reference = value_type&;
        using iterator_category = ::std::forward_iterator_tag;

        using storage_type = typename generator_type::promise_type::storage_type;

    public:
        sync_generator_iterator() noexcept = default;

        sync_generator_iterator(generator_type& g) noexcept
            : m_generator{ &g }
        {
            m_reach_end = !m_generator->move_next();
        }

        sync_generator_iterator(const sync_generator_iterator&) = delete;
        sync_generator_iterator& operator=(const sync_generator_iterator&) = delete;
        sync_generator_iterator(sync_generator_iterator&&) noexcept = delete;
        sync_generator_iterator& operator=(sync_generator_iterator&&) noexcept = delete;

        /*! \brief Call the `move_next()`
         *  But won't take the ownership of the current yield value.
         */
        sync_generator_iterator& operator++()
        {
            m_storage = nullptr;
            m_reach_end = !m_generator->move_next() || !m_generator->has_value();

            return *this;
        }

        void operator++(int) { operator++(); }

        sync_generator_iterator& operator=(sync_generator_iterator_sentinel)
        {
            m_storage = nullptr;
            m_reach_end = true;
        }

        result_type& operator *()
        {
            if (!this->retrive_data())
                throw ::std::out_of_range{ "There're no any data could be used!" };
            return *m_storage;
        }

        result_type* operator ->()
        {
            if (!this->retrive_data())
                throw ::std::out_of_range{ "There're no any data could be used!" };
            return m_storage.get();
        }

        bool reach_end() const noexcept { return m_reach_end; }

    private:
        bool retrive_data()
        {
            if (!m_storage)
            {
                m_storage.swap(m_generator->current_value_storage());
                return bool(m_storage);
            }
            return true;
        }

        template<generator_concept GG>
        friend bool operator != (const sync_generator_iterator<GG>&, const sync_generator_iterator_sentinel&);

    private: 
        generator_type* m_generator{};
        bool m_reach_end{};
        storage_type m_storage;
    };

    template<generator_concept G>
    bool operator!=(const sync_generator_iterator<G>& iter, const sync_generator_iterator_sentinel&)
    {
        return !iter.m_reach_end;
    }
}

KOIOS_NAMESPACE_END

#endif
