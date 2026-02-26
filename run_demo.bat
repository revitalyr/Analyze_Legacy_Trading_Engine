@echo off
echo ================================================================
echo 🚀 SMART POINTER TRADING ENGINE - DEMO EXECUTION
echo ================================================================

cd /d "%~dp0demo"

echo.
echo 🎯 Running Comprehensive Smart Pointer Demo...
echo ================================================================

comprehensive_demo_msvc.exe > demo_output.txt 2>&1

echo.
echo ================================================================
echo 📊 DEMO EXECUTION COMPLETED
echo ================================================================

echo.
echo 📋 Demo output saved to: demo_output.txt
echo.

type demo_output.txt

echo.
echo ================================================================
echo ✅ DEMO RESULTS SUMMARY
echo ================================================================

findstr /C:"✅" demo_output.txt | wc -l > temp_count.txt
set /p success_count=<temp_count.txt
findstr /C:"❌" demo_output.txt | wc -l > temp_count.txt
set /p error_count=<temp_count.txt
del temp_count.txt

echo Success indicators: %success_count%
echo Error indicators: %error_count%

echo.
echo 🎉 Smart Pointer Trading Engine Demo Complete!
echo ================================================================

pause
