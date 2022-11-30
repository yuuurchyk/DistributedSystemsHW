#include <boost/scope_exit.hpp>
#include <gtest/gtest.h>

#include "logger/logger.h"

int main(int argc, char **argv)
{
    logger::setup("future-utils-tests");
    BOOST_SCOPE_EXIT(void)
    {
        logger::teardown();
    }
    BOOST_SCOPE_EXIT_END

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
