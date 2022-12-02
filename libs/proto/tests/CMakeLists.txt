cmake_minimum_required(VERSION 3.22 FATAL_ERROR)

project(proto-tests LANGUAGES CXX)

find_package(GTest 1.12 EXACT REQUIRED)
include(GoogleTest)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS ./**.h ./**.hpp ./**.cpp)

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} PRIVATE proto GTest::gtest)

set_target_properties(
  ${PROJECT_NAME}
  PROPERTIES CXX_STANDARD 17
             CXX_STANDARD_REQUIRED ON
             CXX_EXTENSIONS OFF)

enable_testing()
gtest_discover_tests(${PROJECT_NAME})
