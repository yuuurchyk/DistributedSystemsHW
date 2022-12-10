#include <boost/scope_exit.hpp>

#include "logger/logger.h"

int main()
{
    logger::setup("secondary-node");    // TODO: distinguish between secondary nodes
    BOOST_SCOPE_EXIT(void)
    {
        logger::teardown();
    }
    BOOST_SCOPE_EXIT_END

    LOGI << "Hello";

    return 0;
}
