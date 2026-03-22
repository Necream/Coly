@echo off
setlocal enabledelayedexpansion

:: ===================== 核心配置（不修改源码） =====================
set CURRENT_DIR=%cd%          :: 当前编译目录（你的源码目录）
set COLY_ROOT=C:\Coly         :: 目标安装目录
set SERVICE_DIR=%COLY_ROOT%\VariableSyncService
set TEMP_DIR=%COLY_ROOT%\TempCode
set COMPILER=                 :: 自动识别的编译器（cl/g++）
set EXE_SUFFIX=.exe           :: Windows可执行文件后缀
set SERVER_NEW_NAME=ColyServer :: Server重命名为ColyServer
set CONFIG_FILE=InteractiveColy.cly :: 需要复制的配置文件

:: ===================== 1. 自动识别编译器（仅检测，不修改文件） =====================
echo Step 1/6: Detecting compiler (cl/g++)...

:: 优先检测MSVC的cl.exe（需先运行vcvarsall.bat配置MSVC环境）
where cl > nul 2>&1
if not errorlevel 1 (
    set COMPILER=cl
    echo Detected MSVC cl.exe (priority compiler)
) else (
    :: 检测MinGW的g++
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

:: ===================== 2. 创建目标目录 =====================
echo Step 2/6: Creating target directories...
if not exist "%COLY_ROOT%" mkdir "%COLY_ROOT%"
if not exist "%SERVICE_DIR%" mkdir "%SERVICE_DIR%"
if not exist "%TEMP_DIR%" mkdir "%TEMP_DIR%"
echo Target directories created: %COLY_ROOT%

:: ===================== 3. 编译前检查源码（仅检查存在性，不修改） =====================
echo Step 3/6: Checking source files...
if not exist "%CURRENT_DIR%\Coly.cpp" (
    echo Error: Coly.cpp not found in current directory!
    pause
    exit /b 1
)
if not exist "%CURRENT_DIR%\server.cpp" (
    echo Error: server.cpp not found in current directory!
    pause
    exit /b 1
)
echo All source files found.

:: ===================== 4. 智能编译（仅适配参数，不修改源码） =====================
echo Step 4/6: Compiling with %COMPILER% (C++20)...

:: 4.1 编译Coly.cpp（根据编译器适配参数）
if "%COMPILER%"=="cl" (
    :: MSVC cl编译参数（适配C++20，-I. → /I.，无-lpthread）
    cl /std:c++20 /I. /Fe:"%CURRENT_DIR%\Coly%EXE_SUFFIX%" "%CURRENT_DIR%\Coly.cpp" /link /OUT:"%CURRENT_DIR%\Coly%EXE_SUFFIX%"
) else (
    :: MinGW g++编译参数（C++20，-I.，-lpthread）
    g++ -std=c++20 -I. -lpthread "%CURRENT_DIR%\Coly.cpp" -o "%CURRENT_DIR%\Coly%EXE_SUFFIX%"
)
:: 检查Coly编译结果
if not exist "%CURRENT_DIR%\Coly%EXE_SUFFIX%" (
    echo Error: Failed to compile Coly.cpp with %COMPILER%!
    pause
    exit /b 1
)
echo Compiled: Coly.cpp → %CURRENT_DIR%\Coly%EXE_SUFFIX%

:: 4.2 编译server.cpp（根据编译器适配参数）
if "%COMPILER%"=="cl" (
    :: MSVC cl编译参数
    cl /std:c++20 /I. /Fe:"%CURRENT_DIR%\server%EXE_SUFFIX%" "%CURRENT_DIR%\server.cpp" /link /OUT:"%CURRENT_DIR%\server%EXE_SUFFIX%"
) else (
    :: MinGW g++编译参数
    g++ -std=c++20 -I. -lpthread "%CURRENT_DIR%\server.cpp" -o "%CURRENT_DIR%\server%EXE_SUFFIX%"
)
:: 检查server编译结果
if not exist "%CURRENT_DIR%\server%EXE_SUFFIX%" (
    echo Error: Failed to compile server.cpp with %COMPILER%!
    pause
    exit /b 1
)
echo Compiled: server.cpp → %CURRENT_DIR%\server%EXE_SUFFIX%

:: ===================== 5. 复制编译产物到目标目录（重命名Server为ColyServer） =====================
echo Step 5/6: Copying binaries to target directory...
copy /Y "%CURRENT_DIR%\Coly%EXE_SUFFIX%" "%COLY_ROOT%\" > nul
copy /Y "%CURRENT_DIR%\server%EXE_SUFFIX%" "%SERVICE_DIR%\%SERVER_NEW_NAME%%EXE_SUFFIX%" > nul
echo Copied: Coly%EXE_SUFFIX% → %COLY_ROOT%\Coly%EXE_SUFFIX%
echo Copied & renamed: server%EXE_SUFFIX% → %SERVICE_DIR%\%SERVER_NEW_NAME%%EXE_SUFFIX%

:: ===================== 6. 复制配置文件（InteractiveColy.cly） =====================
echo Step 6/6: Copying configuration file (%CONFIG_FILE%)...
if exist "%CURRENT_DIR%\%CONFIG_FILE%" (
    copy /Y "%CURRENT_DIR%\%CONFIG_FILE%" "%COLY_ROOT%\" > nul
    echo Copied: %CONFIG_FILE% → %COLY_ROOT%\%CONFIG_FILE%
) else (
    echo Error: %CONFIG_FILE% not found in current directory!
    pause
    exit /b 1
)

:: ===================== 完成 =====================
echo.
echo ========================================
echo Coly v1.5.3 Build & Install Success!
echo ========================================
echo Compiler used: %COMPILER%
echo Local binaries: %CURRENT_DIR%\Coly%EXE_SUFFIX%, %CURRENT_DIR%\server%EXE_SUFFIX%
echo Installed to: %COLY_ROOT%\Coly%EXE_SUFFIX%
echo               %SERVICE_DIR%\%SERVER_NEW_NAME%%EXE_SUFFIX%
echo Config file: %COLY_ROOT%\%CONFIG_FILE%
echo Note: No source files were modified!
echo ========================================

pause