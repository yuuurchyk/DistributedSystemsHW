cmake_minimum_required(VERSION 3.22 FATAL_ERROR)

project(future-utils LANGUAGES CXX)

find_package(Boost 1.80 EXACT REQUIRED)
find_package(Threads REQUIRED)

file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS ./include/${PROJECT_NAME}/**.h
     ./include/${PROJECT_NAME}/**.hpp)
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS ./src/**.cpp ./src/**.h
     ./src/**.hpp)

add_library(${PROJECT_NAME} ${HEADERS} ${SOURCES})

target_include_directories(
  ${PROJECT_NAME}
  PUBLIC include
  PRIVATE src)
target_link_libraries(${PROJECT_NAME} PUBLIC Threads::Threads Boost::boost
                                             logger utils)
target_compile_definitions(${PROJECT_NAME} PUBLIC BOOST_THREAD_VERSION=4)

set_target_properties(
  ${PROJECT_NAME}
  PROPERTIES CXX_STANDARD 17
             CXX_STANDARD_REQUIRED ON
             CXX_EXTENSIONS OFF)

if(log-replication-build-tests)
  add_subdirectory(tests)
endif()
