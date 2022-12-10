#include <boost/scope_exit.hpp>

#include "logger/logger.h"

int main()
{
    logger::setup("master-node");
    BOOST_SCOPE_EXIT(void)
    {
        logger::teardown();
    }
    BOOST_SCOPE_EXIT_END

    LOGI << "Hello";

    return 0;
}
