cmake_minimum_required(VERSION 3.16)
project(TradingDemo)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(../include)
include_directories(../../cpp_fixed)

# Add the demo executable
add_executable(trading_demo trading_demo.cpp)

# Link with the orderbook library
target_link_libraries(trading_demo ${CMAKE_CURRENT_SOURCE_DIR}/../build_gtest/orderbook.lib)

# Set include directories for the target
target_include_directories(trading_demo PRIVATE ../include ../../cpp_fixed)

# Add Windows subsystem
target_link_options(trading_demo PRIVATE /SUBSYSTEM:CONSOLE)
