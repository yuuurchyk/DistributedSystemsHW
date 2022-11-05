#include <boost/scope_exit.hpp>

#include "logger/logger.h"

class A : private logger::NumIdEntity<A>
{
public:
    void func() { EN_LOGI << "HELLOU?"; }
};

class B : private logger::StringIdEntity<B>
{
public:
    B() : logger::StringIdEntity<B>{ "sampleId" } {}

    void func() { EN_LOGW << "HELLO????"; }
};

int main()
{
    logger::setup("sample");
    BOOST_SCOPE_EXIT(void)
    {
        logger::teardown();
    }
    BOOST_SCOPE_EXIT_END

    LOGI << "Hello?";

    auto a = A{};
    a.func();

    auto b = B{};
    b.func();

    return 0;
}
