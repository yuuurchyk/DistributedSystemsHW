cmake_minimum_required(VERSION 3.22 FATAL_ERROR)

project(communicationendpoint LANGUAGES CXX)

find_package(Boost 1.80 EXACT REQUIRED)

file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS ./include/${PROJECT_NAME}/**.h
     ./include/${PROJECT_NAME}/**.hpp)
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS ./src/**.cpp ./src/**.h
     ./src/**.hpp)

add_library(${PROJECT_NAME} STATIC ${HEADERS} ${SOURCES})

target_link_libraries(${PROJECT_NAME} PUBLIC protocol Boost::boost logger utils
                                             constants)

target_include_directories(
  ${PROJECT_NAME}
  PUBLIC include
  PRIVATE src)
set_target_properties(
  ${PROJECT_NAME}
  PROPERTIES CXX_STANDARD 20
             CXX_STANDARD_REQUIRED ON
             CXX_EXTENSIONS OFF)
