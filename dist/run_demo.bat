@echo off
echo cpp_orderbook Demo
echo ==================
echo.
echo Running performance benchmark...
bin\benchmark_test.exe
echo.
echo Running exchange tests...
bin\exchange_test.exe
echo.
echo Demo completed. Press any key to exit...
pause > nul
