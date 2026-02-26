cmake_minimum_required(VERSION 3.16)
project(SmartPointerTest)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(../include)
include_directories(../../cpp_fixed)

# Add the test executable
add_executable(simple_test simple_test.cpp)

# Link with the orderbook library
target_link_libraries(simple_test ${CMAKE_CURRENT_SOURCE_DIR}/../build_gtest/orderbook.lib)

# Set include directories for the target
target_include_directories(simple_test PRIVATE ../include ../../cpp_fixed)
