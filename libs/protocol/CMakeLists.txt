cmake_minimum_required(VERSION 3.22 FATAL_ERROR)

project(protocol LANGUAGES CXX)

file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS ./include/${PROJECT_NAME}/**.h
     ./include/${PROJECT_NAME}/**.hpp)
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS ./src/**.cpp ./src/**.h
     ./src/**.hpp)

add_library(${PROJECT_NAME} STATIC ${HEADERS} ${SOURCES})

target_include_directories(
  ${PROJECT_NAME}
  PUBLIC include
  PRIVATE src)
target_link_libraries(${PROJECT_NAME} PUBLIC logger)
