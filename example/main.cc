#include "koios/future.h"
#include <iostream>

class lifetime {
public:
    // Constructor
    lifetime() {
        std::cout << "constructor called. " << std::endl;
    }

    // Copy Constructor
    lifetime(const lifetime& other) {
        std::cout << "copy_constructor called." << std::endl;
    }

    // Move Constructor
    lifetime(lifetime&& other) noexcept {
        std::cout << "move_constructor called." << std::endl;
    }

    // Copy Assignment Operator
    lifetime& operator=(const lifetime& other) {
        std::cout << "copy_assignment_operator called." << std::endl;
        return *this;
    }

    // Move Assignment Operator
    lifetime& operator=(lifetime&& other) noexcept {
        std::cout << "move_assignment_operator called." << std::endl;
        return *this;
    }

    // Destructor
    ~lifetime() {
        std::cout << "destructor called." << std::endl;
    }
};

void func(koios::promise<lifetime> i)
{
    i.set_value(lifetime{});
    i.send();
}

int main()
{
    koios::promise<lifetime> ip{};
    auto f = ip.get_future();
    func(::std::move(ip));
    auto val = f.get();
}
