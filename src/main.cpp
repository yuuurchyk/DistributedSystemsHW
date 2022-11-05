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

class C : private logger::Entity<C>
{
public:
    void func() { EN_LOGE << "this should be critical"; }
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

    auto c = C{};
    c.func();

    return 0;
}
