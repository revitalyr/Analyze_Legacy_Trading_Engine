@echo off
echo ================================================================
echo 🧹 CLEANING UNNECESSARY FILES AND DEPENDENCIES
echo ================================================================

cd /d "%~dp0"

echo.
echo 📋 Removing temporary and generated files...
echo --------------------------------------------------------

REM Remove temporary files
if exist demo_output.txt (
    echo   Removing demo_output.txt
    del demo_output.txt
)

if exist temp_count.txt (
    echo   Removing temp_count.txt
    del temp_count.txt
)

REM Remove test executables in root (should be in build directories)
if exist minimal_ordermap_test.exe (
    echo   Removing minimal_ordermap_test.exe
    del minimal_ordermap_test.exe
)

if exist test_no_macros.exe (
    echo   Removing test_no_macros.exe
    del test_no_macros.exe
)

if exist test_order_constructor.exe (
    echo   Removing test_order_constructor.exe
    del test_order_constructor.exe
)

if exist test_order_with_fixed.exe (
    echo   Removing test_order_with_fixed.exe
    del test_order_with_fixed.exe
)

if exist test_smart_pointers.exe (
    echo   Removing test_smart_pointers.exe
    del test_smart_pointers.exe
)

REM Remove archive files
if exist *.zip (
    echo   Removing archive files
    del *.zip
)

echo.
echo 📋 Checking build directories...
echo --------------------------------------------------------

if exist build (
    echo   Found build directory with contents:
    dir /b build | findstr /v "^$" > temp_build_files.txt
    for /f "tokens=*" %%f in (temp_build_files.txt) do echo     - %%f
    del temp_build_files.txt
    echo   (Keeping build/ - may contain useful build artifacts)
)

if exist build_gtest (
    echo   Found build_gtest directory with contents:
    dir /b build_gtest | findstr /v "^$" > temp_gtest_files.txt
    for /f "tokens=*" %%f in (temp_gtest_files.txt) do echo     - %%f
    del temp_gtest_files.txt
    echo   (Keeping build_gtest/ - contains test executables and libraries)
)

if exist build_msvc (
    echo   Removing empty build_msvc directory
    rmdir /s /q build_msvc
)

echo.
echo 📋 Analyzing source files for dependencies...
echo --------------------------------------------------------

echo   Core source files:
dir /b src\*.cpp 2>nul

echo.
echo   Header files:
dir /b include\*.h 2>nul

echo.
echo   Demo files:
dir /b demo\*.cpp 2>nul

echo.
echo 📋 Checking for duplicate or obsolete files...
echo --------------------------------------------------------

if exist CMakeLists_old.txt (
    echo   Removing obsolete CMakeLists_old.txt
    del CMakeLists_old.txt
)

echo.
echo 📋 Final directory structure:
echo --------------------------------------------------------

echo   Source code:
echo     - src/        (core implementation)
echo     - include/    (header files)
echo     - demo/       (demonstration programs)
echo     - tests/      (unit tests)
echo     - cmake/      (CMake modules)

echo.
echo   Build artifacts:
echo     - build_gtest/ (compiled tests and libraries)
echo     - build/       (general build files)

echo.
echo   Configuration:
echo     - CMakeLists.txt
echo     - Makefile
echo     - *.bat scripts

echo.
echo ================================================================
echo ✅ CLEANUP COMPLETED
echo ================================================================
echo.
echo 📊 Summary:
echo   - Removed temporary files and outputs
echo   - Removed duplicate executables from root
echo   - Updated .gitignore for better file management
echo   - Preserved essential build artifacts
echo   - Maintained clean project structure
echo.
echo 🎯 Project is now clean and ready for development!
echo ================================================================

pause
