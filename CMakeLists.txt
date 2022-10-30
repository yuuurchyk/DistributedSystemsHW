cmake_minimum_required(VERSION 3.22 FATAL_ERROR)

project(logreplication LANGUAGES CXX)
option(${PROJECT_NAME}-build-tests "include tests" ON)

find_package(
  Boost 1.80 EXACT
  COMPONENTS json
  REQUIRED)
find_package(Threads REQUIRED)
find_package(TBB 2021.6.0 EXACT REQUIRED)

add_subdirectory(libs)

foreach(filename client server secondary)
  set(targetName ${filename})

  add_executable(${targetName} src/${filename}.cpp)

  target_link_libraries(${targetName} PRIVATE Boost::boost Boost::json TBB::tbb
                                              ${libsTargets})
  set_target_properties(
    ${targetName}
    PROPERTIES CXX_STANDARD 17
               CXX_STANDARD_REQUIRED ON
               CXX_EXTENSIONS OFF)

  install(TARGETS ${targetName})
endforeach()
