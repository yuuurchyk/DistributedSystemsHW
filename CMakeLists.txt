cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

project(HW1 LANGUAGES CXX)

find_package(
  Boost 1.80 EXACT REQUIRED
  COMPONENTS program_options
  REQUIRED)
find_package(Threads REQUIRED)

add_subdirectory(libs)

foreach(filename client server)
  set(targetName ${PROJECT_NAME}_${filename})

  add_executable(${targetName} src/${filename}.cpp)

  target_link_libraries(
    ${targetName} PRIVATE logger Boost::boost Boost::program_options
                          Threads::Threads)
  set_target_properties(
    ${targetName}
    PROPERTIES CXX_STANDARD 17
               CXX_STANDARD_REQUIRED ON
               CXX_EXTENSIONS OFF)

  install(TARGETS ${targetName})
endforeach()
