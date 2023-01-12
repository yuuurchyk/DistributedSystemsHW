cmake_minimum_required(VERSION 3.22 FATAL_ERROR)

project(utils LANGUAGES CXX)

find_package(Boost 1.80 EXACT REQUIRED COMPONENTS thread)

file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS ./include/${PROJECT_NAME}/**.h
     ./include/${PROJECT_NAME}/**.hpp)
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS ./src/**.cpp ./src/**.h
     ./src/**.hpp)

add_library(${PROJECT_NAME} STATIC ${HEADERS} ${SOURCES})

target_include_directories(${PROJECT_NAME} INTERFACE include)
target_link_libraries(${PROJECT_NAME} PUBLIC Boost::boost Boost::thread)
