@echo off
setlocal enabledelayedexpansion

:: ===================== Core Configuration =====================
set CURRENT_DIR=%cd%
set COLY_ROOT=C:\Coly
set SETTINGS_DIR=%COLY_ROOT%\Settings
set SERVICE_DIR=%COLY_ROOT%\VariableSyncService
set LIB_DIR=%COLY_ROOT%\VariableSyncLib
set TEMP_DIR=%COLY_ROOT%\TempCode
set SERVER_NEW_NAME=ColyServer
set CONFIG_FILE=InteractiveColy.cly
set LANGSYNC_DIR=%CURRENT_DIR%\LanguageSyncLib

:: ===================== 1. Detect Compiler =====================
echo Step 1/7: Detecting compiler (cl/g++)...

where cl > nul 2>&1
if not errorlevel 1 (
    set COMPILER=cl
    echo Detected MSVC cl.exe
) else (
    where g++ > nul 2>&1
    if not errorlevel 1 (
        set COMPILER=g++
        echo Detected MinGW g++
    ) else (
        echo Error: No compiler found!
        echo Please install MSVC (cl.exe) or MinGW (g++), and add to PATH.
        pause
        exit /b 1
    )
)

:: ===================== 2. Create Target Directories =====================
echo Step 2/7: Creating target directories...
if not exist "%COLY_ROOT%" mkdir "%COLY_ROOT%"
if not exist "%SETTINGS_DIR%" mkdir "%SETTINGS_DIR%"
if not exist "%SERVICE_DIR%" mkdir "%SERVICE_DIR%"
if not exist "%LIB_DIR%" mkdir "%LIB_DIR%"
if not exist "%TEMP_DIR%" mkdir "%TEMP_DIR%"
echo Target directories created.

:: ===================== 3. Check Source Files =====================
echo Step 3/7: Checking source files...
if not exist "%CURRENT_DIR%\Coly.cpp" (
    echo Error: Coly.cpp not found!
    pause
    exit /b 1
)
if not exist "%CURRENT_DIR%\server.cpp" (
    echo Error: server.cpp not found!
    pause
    exit /b 1
)
if not exist "%CURRENT_DIR%\%CONFIG_FILE%" (
    echo Error: %CONFIG_FILE% not found!
    pause
    exit /b 1
)
echo All source files found.

:: ===================== 4. Compile =====================
echo Step 4/7: Compiling with %COMPILER% (C++20)...

if "%COMPILER%"=="cl" (
    cl /std:c++20 /I. /Fe:"%CURRENT_DIR%\Coly.exe" "%CURRENT_DIR%\Coly.cpp" /link /OUT:"%CURRENT_DIR%\Coly.exe"
) else (
    g++ -std=c++20 -I. -lpthread "%CURRENT_DIR%\Coly.cpp" -o "%CURRENT_DIR%\Coly.exe"
)
if not exist "%CURRENT_DIR%\Coly.exe" (
    echo Error: Failed to compile Coly.cpp!
    pause
    exit /b 1
)
echo Compiled: Coly.exe

if "%COMPILER%"=="cl" (
    cl /std:c++20 /I. /Fe:"%CURRENT_DIR%\server.exe" "%CURRENT_DIR%\server.cpp" /link /OUT:"%CURRENT_DIR%\server.exe"
) else (
    g++ -std=c++20 -I. -lpthread "%CURRENT_DIR%\server.cpp" -o "%CURRENT_DIR%\server.exe"
)
if not exist "%CURRENT_DIR%\server.exe" (
    echo Error: Failed to compile server.cpp!
    pause
    exit /b 1
)
echo Compiled: server.exe

:: ===================== 5. Copy Binaries =====================
echo Step 5/7: Copying binaries...
copy /Y "%CURRENT_DIR%\Coly.exe" "%COLY_ROOT%\" > nul
copy /Y "%CURRENT_DIR%\server.exe" "%SERVICE_DIR%\%SERVER_NEW_NAME%.exe" > nul
echo Copied: Coly.exe, %SERVER_NEW_NAME%.exe

:: ===================== 6. Copy Libraries and Config =====================
echo Step 6/7: Copying libraries and configuration...

:: VariableSyncLib headers
if exist "%CURRENT_DIR%\LanguageSyncLib\json.hpp" copy /Y "%CURRENT_DIR%\LanguageSyncLib\json.hpp" "%LIB_DIR%\" > nul
if exist "%CURRENT_DIR%\LanguageSyncLib\GXPass.hpp" copy /Y "%CURRENT_DIR%\LanguageSyncLib\GXPass.hpp" "%LIB_DIR%\" > nul
if exist "%CURRENT_DIR%\LanguageSyncLib\ColyCppSync.hpp" copy /Y "%CURRENT_DIR%\LanguageSyncLib\ColyCppSync.hpp" "%LIB_DIR%\" > nul
if exist "%CURRENT_DIR%\LanguageSyncLib\VariableSyncService.hpp" copy /Y "%CURRENT_DIR%\LanguageSyncLib\VariableSyncService.hpp" "%LIB_DIR%\" > nul
if exist "%CURRENT_DIR%\NCInt.hpp" copy /Y "%CURRENT_DIR%\NCInt.hpp" "%LIB_DIR%\" > nul
if exist "%CURRENT_DIR%\NCint.hpp" copy /Y "%CURRENT_DIR%\NCint.hpp" "%LIB_DIR%\" > nul
if exist "%CURRENT_DIR%\asio.hpp" copy /Y "%CURRENT_DIR%\asio.hpp" "%LIB_DIR%\" > nul
if exist "%CURRENT_DIR%\asio" (
    if not exist "%LIB_DIR%\asio" mkdir "%LIB_DIR%\asio"
    xcopy /E /I /Y "%CURRENT_DIR%\asio" "%LIB_DIR%\asio\" > nul
)
echo Copied: VariableSyncLib headers

:: LanguageMap
if exist "%CURRENT_DIR%\Settings\LanguageMap_Windows.json" (
    copy /Y "%CURRENT_DIR%\Settings\LanguageMap_Windows.json" "%SETTINGS_DIR%\LanguageMap.json" > nul
    echo Copied: LanguageMap.json (from LanguageMap_Windows.json)
)

:: InteractiveColy.cly
copy /Y "%CURRENT_DIR%\%CONFIG_FILE%" "%COLY_ROOT%\" > nul
echo Copied: %CONFIG_FILE%

:: ===================== 7. Install ColyPythonSync =====================
echo Step 7/7: Installing ColyPythonSync...
if exist "%LANGSYNC_DIR%\ColyPythonSync\pyproject.toml" (
    pushd "%LANGSYNC_DIR%\ColyPythonSync"
    pip install .
    popd
    echo ColyPythonSync installed.
) else (
    echo Warning: ColyPythonSync package not found, skipping.
)

:: ===================== Complete =====================
echo.
echo ========================================
echo Coly v2.0.3 Install Success!
echo ========================================
echo Compiler: %COMPILER%
echo Installed to: %COLY_ROOT%
echo Run: %COLY_ROOT%\Coly.exe
echo Server: %SERVICE_DIR%\%SERVER_NEW_NAME%.exe
echo ========================================

pause
