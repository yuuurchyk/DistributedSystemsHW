cmake_minimum_required(VERSION 3.22 FATAL_ERROR)

project(constants2 LANGUAGES CXX)

file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS ./include/${PROJECT_NAME}/**.h
     ./include/${PROJECT_NAME}/**.hpp)

add_library(${PROJECT_NAME} INTERFACE ${HEADERS})

target_include_directories(${PROJECT_NAME} INTERFACE include)
