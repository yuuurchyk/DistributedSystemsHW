#include <thread>

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

    auto t = std::thread{ []()
                          {
                              for (int i = 0; i < 2000; ++i)
                                  LOGW << "HELLO from thread, " << i;
                          } };

    LOGI << "Hello?";

    auto a = A{};
    a.func();

    auto b = B{};
    b.func();

    auto c = C{};
    c.func();

    std::this_thread::sleep_for(std::chrono::seconds{ 2 });
    t.join();

    c.func();

    return 0;
}
