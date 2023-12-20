#include "koios/future.h"

void func(koios::promise<int> i)
{
    i.set_value(1);
    i.send();
}

int main()
{
    koios::promise<int> ip{};
    auto f = ip.get_future();
    func(::std::move(ip));
    if (f.ready())
    {
        ::std::cout << f.get() << ::std::endl;
    }
}
