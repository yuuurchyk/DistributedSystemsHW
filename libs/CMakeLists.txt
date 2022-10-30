add_subdirectory(utils)
add_subdirectory(logger)
add_subdirectory(protocol)
add_subdirectory(communicationendpoint)
add_subdirectory(iocontextpool)
add_subdirectory(socketacceptor)

set(libsTargets
    "utils;logger;protocol;communicationendpoint;iocontextpool;socketacceptor"
    PARENT_SCOPE)
