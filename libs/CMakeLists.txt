add_subdirectory(utils)
add_subdirectory(logger)
add_subdirectory(protocol)
add_subdirectory(messagesstorage)
add_subdirectory(communicationendpoint)

set(libsTargets
    "utils;logger;protocol;messagesstorage;communicationendpoint"
    PARENT_SCOPE)
