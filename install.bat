@echo off
:: Coly v1.5.3 Full Install & Build Script (Windows)
:: Auto-install MinGW (optional), create directories, compile with C++20
:: Usage: Run as Administrator

setlocal enabledelayedexpansion

:: 1. Define core paths
set COLY_ROOT=C:\Coly
set SRC_DIR=%COLY_ROOT%\src
set LIB_DIR=%COLY_ROOT%\VariableSyncLib
set SETTINGS_DIR=%COLY_ROOT%\Settings
set SERVICE_DIR=%COLY_ROOT%\VariableSyncService
set TEMP_DIR=%COLY_ROOT%\TempCode
set OUTPUT_EXE=%COLY_ROOT%\coly.exe
set SERVICE_EXE=%SERVICE_DIR%\server.exe

:: 2. Create all required directories
echo Creating directory structure...
for %%d in ("%COLY_ROOT%" "%SRC_DIR%" "%LIB_DIR%" "%SETTINGS_DIR%" "%SERVICE_DIR%" "%TEMP_DIR%") do (
    if not exist "%%d" mkdir "%%d"
    if not exist "%%d" (
        echo Error: Failed to create directory %%d
        pause
        exit /b 1
    )
)

:: 3. Check for MinGW (g++) - skip auto-install (manual install recommended)
echo Checking for g++ (MinGW-w64)...
where g++ > nul 2>&1
if errorlevel 1 (
    echo Error: g++ not found. Please install MinGW-w64 (with C++20 support) and add to PATH.
    pause
    exit /b 1
)

:: 4. Create default source files (replace with actual code)
echo Creating default source files...
echo #include <iostream> > "%SRC_DIR%\main.cpp"
echo int main(int argc, char* argv[]) { >> "%SRC_DIR%\main.cpp"
echo     std::cout << "Coly v1.5.3 - Windows Build" << std::endl; >> "%SRC_DIR%\main.cpp"
echo     return 0; >> "%SRC_DIR%\main.cpp"
echo } >> "%SRC_DIR%\main.cpp"

echo #include <iostream> > "%SERVICE_DIR%\server.cpp"
echo int main(int argc, char* argv[]) { >> "%SERVICE_DIR%\server.cpp"
echo     std::cout << "Coly VariableSyncService - Running" << std::endl; >> "%SERVICE_DIR%\server.cpp"
echo     while(true) { Sleep(1000); } >> "%SERVICE_DIR%\server.cpp"
echo     return 0; >> "%SERVICE_DIR%\server.cpp"
echo } >> "%SERVICE_DIR%\server.cpp"

:: 5. Create placeholder library headers
echo Creating library placeholders...
type nul > "%LIB_DIR%\json.hpp"
type nul > "%LIB_DIR%\GXPass.hpp"
type nul > "%LIB_DIR%\NCInt.hpp"
type nul > "%LIB_DIR%\asio.hpp"

:: 6. Compile with C++20 standard
echo Compiling Coly core (C++20)...
g++ -std=c++20 ^
    -I. ^
    -I"%LIB_DIR%" ^
    "%SRC_DIR%\main.cpp" ^
    -o "%OUTPUT_EXE%"

if not exist "%OUTPUT_EXE%" (
    echo Error: Failed to compile Coly core executable
    pause
    exit /b 1
)

echo Compiling VariableSyncService (C++20)...
g++ -std=c++20 ^
    -I. ^
    -I"%LIB_DIR%" ^
    "%SERVICE_DIR%\server.cpp" ^
    -o "%SERVICE_EXE%"

if not exist "%SERVICE_EXE%" (
    echo Error: Failed to compile VariableSyncService
    pause
    exit /b 1
)

:: 7. Generate LanguageMap_Windows.json (C++20)
echo Generating LanguageMap configuration...
echo {^
    "C++": "g++ -std=c++20 -I%LIB_DIR% $SRC -o $OUT",^
    "Python": "python $SRC"^
} > "%SETTINGS_DIR%\LanguageMap_Windows.json"

:: 8. Create startup batch file
echo @echo off > "%COLY_ROOT%\start-service.bat"
echo start %SERVICE_EXE% >> "%COLY_ROOT%\start-service.bat"
echo echo Coly VariableSyncService started >> "%COLY_ROOT%\start-service.bat"

:: 9. Final verification
echo ========================================
echo Coly v1.5.3 Install & Build Complete
echo ========================================
echo Core executable: %OUTPUT_EXE%
echo Service executable: %SERVICE_EXE%
echo Temp directory: %TEMP_DIR%
echo LanguageMap: %SETTINGS_DIR%\LanguageMap_Windows.json
echo Start service: %COLY_ROOT%\start-service.bat
echo Run Coly: %OUTPUT_EXE%

pause