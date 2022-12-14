cmake_minimum_required(VERSION 3.22 FATAL_ERROR)

project(secondary-node LANGUAGES CXX)

find_package(
  Boost 1.80 EXACT
  COMPONENTS json program_options
  REQUIRED)
find_package(TBB 2021.6.0 EXACT REQUIRED)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS ./**.cpp ./**.h ./**.hpp)

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(
  ${PROJECT_NAME} PRIVATE TBB::tbb Boost::json Boost::program_options
                          ${libsTargets})

set_target_properties(
  ${PROJECT_NAME}
  PROPERTIES CXX_STANDARD 20
             CXX_STANDARD_REQUIRED ON
             CXX_EXTENSIONS OFF)
target_include_directories(${PROJECT_NAME} PRIVATE .)

install(TARGETS ${PROJECT_NAME})
