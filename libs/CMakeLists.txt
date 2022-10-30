add_subdirectory(utils)
add_subdirectory(logger)
add_subdirectory(protocol)
add_subdirectory(communicationendpoint)
add_subdirectory(iocontextpool)

set(libsTargets
    "utils;logger;protocol;communicationendpoint;iocontextpool"
    PARENT_SCOPE)
