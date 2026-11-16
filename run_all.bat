@echo off
REM ==========================================================
REM  DAA Assignment - compile and run all experiments (Windows)
REM  Needs g++ (MinGW-w64) on PATH. Just double-click this file.
REM  Everything printed is also saved to output_log.txt
REM ==========================================================
cd /d "%~dp0"
echo Running all experiments, please wait (about 1-2 minutes)...
call :main > output_log.txt 2>&1
echo.
echo Finished. Results are in the CSV files and output_log.txt
echo ALL_DONE > done.txt
exit /b

:main
echo ===== System information =====
powershell -NoProfile -Command "(Get-CimInstance Win32_Processor).Name; $os = Get-CimInstance Win32_OperatingSystem; $os.Caption + ' (build ' + $os.BuildNumber + ')'; [math]::Round((Get-CimInstance Win32_ComputerSystem).TotalPhysicalMemory/1GB,1).ToString() + ' GB RAM'; 'Power plugged in: ' + ((Get-CimInstance -ClassName BatteryStatus -Namespace root\wmi -ErrorAction SilentlyContinue).PowerOnline)"
echo.
echo ===== Tools =====
where g++
g++ --version
call code --version
echo.
for %%f in (insertion_sort merge_sort quick_sort floyd_warshall kmp sort_crossover) do (
    g++ -O2 -std=c++17 %%f.cpp -o %%f.exe
    if errorlevel 1 echo COMPILE FAILED: %%f
)
for %%f in (insertion_sort merge_sort quick_sort floyd_warshall kmp sort_crossover) do (
    echo.
    echo ===================== %%f =====================
    %%f.exe
)
echo.
echo ===== All experiments finished =====
exit /b
