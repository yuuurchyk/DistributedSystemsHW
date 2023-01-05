set(libsTargets "")
macro(add_lib_target subdirectoryName)
  add_subdirectory(${subdirectoryName})
  list(APPEND libsTargets ${subdirectoryName})
endmacro()

add_lib_target(utils)
add_lib_target(logger)
add_lib_target(net-utils)
add_lib_target(reflection)
add_lib_target(proto)
add_lib_target(proto2)

set(libsTargets
    ${libsTargets}
    PARENT_SCOPE)
