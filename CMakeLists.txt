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

foreach(appname secondary)
  file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/${appname}/**.cpp
       src/${appname}/**.h src/${appname}/**.hpp)
  add_executable(${appname} ${SOURCES})

  target_link_libraries(${appname} PRIVATE Boost::boost Boost::json TBB::tbb
                                           ${libsTargets})
  target_include_directories(${appname} PRIVATE src/${appname})
  set_target_properties(
    ${appname}
    PROPERTIES CXX_STANDARD 17
               CXX_STANDARD_REQUIRED ON
               CXX_EXTENSIONS OFF)

  install(TARGETS ${appname})
endforeach()

add_executable(client src/client.cpp)
target_link_libraries(client PRIVATE Boost::boost Boost::json TBB::tbb
                                     ${libsTargets})
set_target_properties(
  client
  PROPERTIES CXX_STANDARD 17
             CXX_STANDARD_REQUIRED ON
             CXX_EXTENSIONS OFF)
install(TARGETS client)
