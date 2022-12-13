cmake_minimum_required(VERSION 3.22 FATAL_ERROR)

project(log-replication LANGUAGES CXX)
option(${PROJECT_NAME}-build-tests "include tests" ON)

add_subdirectory(libs)
add_subdirectory(app)
