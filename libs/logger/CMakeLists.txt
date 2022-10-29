cmake_minimum_required(VERSION 3.22 FATAL_ERROR)

project(logger LANGUAGES CXX)

find_package(Boost 1.80 EXACT REQUIRED COMPONENTS log log_setup)

file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS ./include/${PROJECT_NAME}/**.h
     ./include/${PROJECT_NAME}/**.hpp)
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS ./src/**.cpp ./src/**.h
     ./src/**.hpp)

add_library(${PROJECT_NAME} STATIC ${HEADERS} ${SOURCES})

target_compile_definitions(${PROJECT_NAME} PUBLIC -DBOOST_LOG_DYN_LINK)
target_include_directories(
  ${PROJECT_NAME}
  PUBLIC include
  PRIVATE src)
target_link_libraries(${PROJECT_NAME} PUBLIC Boost::log Boost::log_setup utils)

set_target_properties(
  ${PROJECT_NAME}
  PROPERTIES CXX_STANDARD 17
             CXX_STANDARD_REQUIRED ON
             CXX_EXTENSIONS OFF)

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
