#ifndef KOIOS_ERROR_CATEGORY_H
#define KOIOS_ERROR_CATEGORY_H

#include "koios/macros.h"

#include <string>
#include <system_error>

KOIOS_NAMESPACE_BEG

class expected_category : public ::std::error_category
{
public:
    virtual const char* name() const noexcept override;
    virtual ~expected_category() noexcept {}
    virtual ::std::string message(int condition) const override;
};

KOIOS_NAMESPACE_END

#endif
