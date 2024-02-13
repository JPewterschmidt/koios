#ifndef KOIOS_IOURING_OP_PERIPHERAL_H
#define KOIOS_IOURING_OP_PERIPHERAL_H

#include <vector>
#include <concepts>
#include <memory>
#include "toolpex/move_only.h"

namespace koios::uring
{

class op_peripheral : public toolpex::move_only
{
public:
    constexpr op_peripheral() noexcept = default;

    class data_interface 
    { 
    public:
        constexpr data_interface() noexcept = default;
        data_interface(data_interface&&) noexcept = default;
        data_interface& operator=(data_interface&&) noexcept = default;
        data_interface(const data_interface&) = delete;
    };

    template<::std::derived_from<data_interface> DataT, typename... Args>
    DataT* add(Args&&... args)
    {
        DataT* result{};
        m_peripheral_datas.emplace_back(result = new DataT(::std::forward<Args>(args)...));
        return result;
    }

private:
    ::std::vector<::std::unique_ptr<data_interface>> m_peripheral_datas;
};

} // namespace koios::uring

#endif
