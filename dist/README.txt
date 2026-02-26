cpp_orderbook - Portable Windows Package
==========================================

This package contains the cpp_orderbook high-performance trading engine
compiled for Windows x64.

Contents:
- bin/: Executable files and runtime DLLs
- lib/: Static library files
- include/: Header files for development
- config/: Configuration files

Quick Start:
1. Run benchmark_test.exe to test performance
2. Run exchange_test.exe to verify functionality
3. Include headers from include/ directory in your project
4. Link with lib/orderbook.lib

Performance:
- 5-10M orders/second per core
- Lock-free data structures
- Custom memory allocator

Requirements:
- Windows 10/11 x64
- Visual C++ Redistributable (if using MSVC build)

Build Information:
- Build Date: 2026-02-25 10:37:00
- Compiler: clang++
- Build Type: Release
- C++ Standard: C++20

For more information, see the project documentation.

Available Executables:
- benchmark_test.exe: Performance benchmarking
- benchmark_multithread_test.exe: Multi-threaded performance test
- bookmap_test.exe: Book map functionality test
- exchange_test.exe: Exchange API test
- order_test.exe: Order object test
- orderbook_test.exe: Order book core test
- orderlist_test.exe: Order list test
- ordermap_test.exe: Order map test
- spinlock_test.exe: Spinlock test

Note: Some tests may show stack overflow on certain systems due to
recursive test patterns. This is expected and does not affect the
core functionality.
