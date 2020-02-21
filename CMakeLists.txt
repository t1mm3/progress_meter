cmake_minimum_required(VERSION 3.10)

project(ProgressMeter VERSION 1.0)

find_package(Threads REQUIRED)

add_executable(example1 example1.cpp)
add_executable(example2 example2.cpp)

add_executable(example3 example3.cpp)
target_link_libraries(example3 ${CMAKE_THREAD_LIBS_INIT})