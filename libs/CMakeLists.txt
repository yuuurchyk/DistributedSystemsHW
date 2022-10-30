add_subdirectory(utils)
add_subdirectory(logger)
add_subdirectory(protocol)
add_subdirectory(communicationendpoint)
add_subdirectory(iocontextpool)
add_subdirectory(socketacceptor)
add_subdirectory(constants)
add_subdirectory(httpsession)

set(libsTargets
    "utils;logger;protocol;communicationendpoint;iocontextpool;socketacceptor;constants;httpsession"
    PARENT_SCOPE)
