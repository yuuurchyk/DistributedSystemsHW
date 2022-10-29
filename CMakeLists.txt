cmake_minimum_required(VERSION 3.22 FATAL_ERROR)

project(HW1 LANGUAGES CXX)
option(${PROJECT_NAME}-build-tests "include tests" ON)

find_package(Boost 1.80 EXACT REQUIRED)
find_package(Threads REQUIRED)

add_subdirectory(libs)

foreach(filename client server)
  set(targetName ${PROJECT_NAME}_${filename})

  add_executable(${targetName} src/${filename}.cpp)

  target_link_libraries(${targetName} PRIVATE Boost::boost ${libsTargets})
  set_target_properties(
    ${targetName}
    PROPERTIES CXX_STANDARD 17
               CXX_STANDARD_REQUIRED ON
               CXX_EXTENSIONS OFF)

  install(TARGETS ${targetName})
endforeach()
