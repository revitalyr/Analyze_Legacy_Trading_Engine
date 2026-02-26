cmake_minimum_required(VERSION 3.20)
project(cpp_orderbook VERSION 2.0.0 LANGUAGES CXX)

# Modern C++ project configuration
cmake_policy(SET CMP0167 NEW)  # FindBoost module policy
cmake_policy(SET CMP0135 NEW)  # Download toolchain policy

# C++ standard configuration
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build type configuration
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler-specific options with modern flags
if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    add_compile_options(
        /W4                    # High warning level
        /permissive-           # Strict standards conformance
        /Zc:__cplusplus        # Correct __cplusplus macro
        /utf-8                 # Source file encoding
    )
    add_compile_definitions(
        _CRT_SECURE_NO_WARNINGS
        WIN32_LEAN_AND_MEAN
        NOMINMAX
    )
else()
    add_compile_options(
        -Wall                  # All warnings
        -Wextra               # Extra warnings
        -Wpedantic            # Pedantic warnings
        -Wconversion          # Implicit conversion warnings
        -Wsign-conversion     # Sign conversion warnings
        -Wcast-align          # Cast alignment warnings
        -Wnull-dereference     # Null dereference warnings
        -Wdouble-promotion    # Double promotion warnings
        -Wold-style-cast      # Old-style cast warnings
        -Woverloaded-virtual  # Overloaded virtual function warnings
        -Wno-microsoft-cpp-macro
        -Wno-language-extension-token
        -Wno-unused-variable
        -Wno-unused-but-set-variable
    )
endif()

# Debug/Release configuration with modern flags
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
        add_compile_options(/Od /Zi /RTC1)
        add_link_options(/DEBUG)
    else()
        add_compile_options(
            -g3                    # Maximum debug information
            -O0                    # No optimization
            -fno-omit-frame-pointer # Better stack traces
        )
    endif()
else()
    if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
        add_compile_options(/O2 /Ob2)
        add_link_options(/LTCG)  # Link-time code generation
    else()
        add_compile_options(
            -O3                    # Aggressive optimization
            -march=native          # Target-specific optimizations
            -flto                  # Link-time optimization
            -DNDEBUG              # Disable debug assertions
        )
        add_link_options(-flto)
    endif()
endif()

# Test framework (GoogleTest)
find_package(GTest CONFIG QUIET)
if(NOT GTest_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
endif()

# Suppress warnings for GoogleTest headers
if(TARGET gtest)
    target_compile_options(gtest PRIVATE -Wno-double-promotion)
    target_compile_options(gtest PRIVATE -Wno-sign-conversion)
    target_compile_options(gtest PRIVATE -Wno-conversion)
    target_compile_options(gtest PRIVATE -Wno-old-style-cast)
endif()

if(TARGET gmock)
    target_compile_options(gmock PRIVATE -Wno-double-promotion)
    target_compile_options(gmock PRIVATE -Wno-sign-conversion)
    target_compile_options(gmock PRIVATE -Wno-conversion)
    target_compile_options(gmock PRIVATE -Wno-old-style-cast)
endif()

include(GoogleTest)

# Include directories with modern organization
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../cpp_fixed)

# Core library sources
set(LIB_SOURCES
    src/orderbook.cpp
    src/exchange.cpp
    src/production_safety.cpp
    src/production_safety_inline.cpp
)

# Modern library configuration
add_library(orderbook STATIC ${LIB_SOURCES})

# Target-specific configuration
target_include_directories(orderbook PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../cpp_fixed>
)

# Compiler-specific target options
if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    target_compile_options(orderbook PRIVATE /W4)
else()
    target_compile_options(orderbook PRIVATE -Wall -Wextra -pedantic -Wno-sign-conversion -Wno-shorten-64-to-32 -Wno-old-style-cast -Wno-unused-parameter)
endif()

# Modern testing configuration
enable_testing()

# Unit tests
file(GLOB UNIT_TEST_SOURCES "tests/unit/*_test.cpp")
foreach(TEST_SOURCE ${UNIT_TEST_SOURCES})
    get_filename_component(TEST_NAME ${TEST_SOURCE} NAME_WE)
    
    add_executable(${TEST_NAME} ${TEST_SOURCE})
    target_link_libraries(${TEST_NAME} 
        PRIVATE orderbook
        PRIVATE GTest::gtest_main
    )
    
    # Suppress warnings for test targets
    target_compile_options(${TEST_NAME} PRIVATE -Wno-sign-conversion -Wno-shorten-64-to-32 -Wno-old-style-cast -Wno-unused-parameter)
 
    gtest_discover_tests(${TEST_NAME}
        TIMEOUT 300
        LABELS "unit"
    )
endforeach()

# Fixed-point arithmetic tests
add_executable(fixed_test ../cpp_fixed/fixed_test.cpp)
target_link_libraries(fixed_test 
    PRIVATE orderbook 
    PRIVATE GTest::gtest_main
)
# Suppress warnings for test targets
target_compile_options(fixed_test PRIVATE -Wno-sign-conversion -Wno-shorten-64-to-32 -Wno-old-style-cast -Wno-unused-parameter)
gtest_discover_tests(fixed_test
        TIMEOUT 300
        LABELS "unit;fixed"
    )

# Benchmark tests
file(GLOB BENCHMARK_TEST_SOURCES "tests/benchmark/*_test.cpp")
foreach(BENCH_SOURCE ${BENCHMARK_TEST_SOURCES})
    get_filename_component(BENCH_NAME ${BENCH_SOURCE} NAME_WE)
    
    add_executable(${BENCH_NAME} ${BENCH_SOURCE})
    target_link_libraries(${BENCH_NAME} 
        PRIVATE orderbook 
    )
    
    # Suppress warnings for benchmark targets
    target_compile_options(${BENCH_NAME} PRIVATE -Wno-sign-conversion -Wno-shorten-64-to-32 -Wno-old-style-cast -Wno-unused-parameter -Wno-implicit-int-float-conversion)
    
    # Add benchmark to CTest
    add_test(NAME ${BENCH_NAME} COMMAND ${BENCH_NAME})
    
    # Set benchmark properties
    set_tests_properties(${BENCH_NAME} PROPERTIES
        TIMEOUT 600
        LABELS "benchmark;performance"
    )
endforeach()

# Modern installation configuration
include(GNUInstallDirs)

install(TARGETS orderbook
    EXPORT orderbook-targets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Install headers with proper organization
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# Install examples
install(DIRECTORY examples/ DESTINATION ${CMAKE_INSTALL_DATADIR}/cpp_orderbook/examples)

# Create and install package configuration
include(CMakePackageConfigHelpers)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/orderbookConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/orderbookConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/orderbook
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/orderbookConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/orderbookConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/orderbookConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/orderbook
)

install(EXPORT orderbook-targets
    FILE orderbook-targets.cmake
    NAMESPACE orderbook::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/orderbook
)

# Custom targets for development workflow
add_custom_target(run_unit_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure --label-regex "unit"
    DEPENDS orderbook
    COMMENT "Running unit tests only"
)

add_custom_target(run_benchmarks
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure --label-regex "benchmark"
    DEPENDS orderbook
    COMMENT "Running benchmark tests only"
)

add_custom_target(run_all_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    DEPENDS orderbook
    COMMENT "Running all tests"
)

# Code analysis and formatting targets
find_program(CLANG_TIDY NAMES clang-tidy)
find_program(CLANG_FORMAT NAMES clang-format)

if(CLANG_TIDY)
    add_custom_target(clang-tidy
        COMMAND ${CLANG_TIDY} -p ${CMAKE_BINARY_DIR} ${LIB_SOURCES}
        COMMENT "Running clang-tidy static analysis"
    )
endif()

if(CLANG_FORMAT)
    add_custom_target(format-code
        COMMAND ${CLANG_FORMAT} -i ${LIB_SOURCES}
        COMMAND ${CLANG_FORMAT} -i ${UNIT_TEST_SOURCES}
        COMMAND ${CLANG_FORMAT} -i ${BENCHMARK_TEST_SOURCES}
        COMMENT "Formatting source code with clang-format"
    )
endif()

# Documentation generation with Doxygen
find_package(Doxygen QUIET)
if(DOXYGEN_FOUND)
    set(DOXYGEN_INPUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
    set(DOXYGEN_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/docs)
    
    add_custom_target(docs
        COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
    )
endif()

# Performance profiling support
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    find_program(PERF NAMES perf)
    if(PERF)
        add_custom_target(profile_benchmarks
            COMMAND ${PERF} record -g ${CMAKE_BINARY_DIR}/benchmark_test
            COMMAND ${PERF} report
            COMMENT "Profiling benchmarks with perf"
        )
    endif()
endif()

# Package configuration
set(CPACK_PACKAGE_NAME "cpp_orderbook")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Modern C++ High-Frequency Trading Engine")
set(CPACK_PACKAGE_VENDOR "Trading Engine Team")
set(CPACK_PACKAGE_CONTACT "trading@example.com")

include(CPack)

# Configuration summary
message(STATUS "")
message(STATUS "=== cpp_orderbook Configuration Summary ===")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "GoogleTest found: ${GTest_FOUND}")
message(STATUS "Install prefix: ${CMAKE_INSTALL_PREFIX}")
list(LENGTH UNIT_TEST_SOURCES UNIT_TEST_SOURCES_LEN)
list(LENGTH BENCHMARK_TEST_SOURCES BENCHMARK_TEST_SOURCES_LEN)
message(STATUS "Unit tests found: ${UNIT_TEST_SOURCES_LEN}")
message(STATUS "Benchmark tests found: ${BENCHMARK_TEST_SOURCES_LEN}")
message(STATUS "Code analysis: ${CLANG_TIDY_FOUND}")
message(STATUS "Code formatting: ${CLANG_FORMAT_FOUND}")
message(STATUS "Documentation: ${DOXYGEN_FOUND}")
message(STATUS "=============================================")
message(STATUS "")
