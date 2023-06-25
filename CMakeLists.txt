cmake_minimum_required(VERSION 3.0.0)
project(allocator_study VERSION 0.1.0)

include(CTest)
enable_testing()

add_executable(allocator_study main.cpp)

add_executable(memory_primitive memory_primitive.cpp)

add_subdirectory(allocator)
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
