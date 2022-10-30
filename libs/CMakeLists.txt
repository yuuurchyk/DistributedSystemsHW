add_subdirectory(utils)
add_subdirectory(logger)
add_subdirectory(protocol)
add_subdirectory(communicationendpoint)
add_subdirectory(iocontextpool)
add_subdirectory(socketacceptor)
add_subdirectory(constants)

set(libsTargets
    "utils;logger;protocol;communicationendpoint;iocontextpool;socketacceptor;constants"
    PARENT_SCOPE)
