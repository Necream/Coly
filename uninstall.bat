@echo off
setlocal enabledelayedexpansion

:: ===================== 核心路径（和最终版install.bat完全匹配） =====================
set COLY_ROOT=C:\Coly         :: 仅删除安装目录，不碰你的源码目录
set START_SCRIPT=%COLY_ROOT%\start-service.bat
set SERVER_NEW_NAME=ColyServer :: 匹配安装脚本的ColyServer命名

:: ===================== 1. 停止运行中的Coly进程（精准匹配） =====================
echo Stopping Coly services...
:: 终止ColyServer.exe（重命名后的服务端）
taskkill /f /im %SERVER_NEW_NAME%.exe /fi "IMAGENAME eq %SERVER_NEW_NAME%.exe" /fi "PATH eq %COLY_ROOT%\VariableSyncService\*" > nul 2>&1
:: 终止Coly.exe
taskkill /f /im coly.exe /fi "IMAGENAME eq coly.exe" /fi "PATH eq %COLY_ROOT%\*" > nul 2>&1
:: 兜底清理残留进程（兼容旧命名server.exe）
taskkill /f /im server.exe > nul 2>&1
taskkill /f /im coly.exe > nul 2>&1
taskkill /f /im %SERVER_NEW_NAME%.exe > nul 2>&1
echo Coly processes stopped.

:: ===================== 2. 移除启动脚本（仅安装目录的） =====================
echo Removing startup script...
if exist "%START_SCRIPT%" (
    del /f /q "%START_SCRIPT%"
    echo Removed: %START_SCRIPT%
) else (
    echo File not found: %START_SCRIPT% (skipping)
)

:: ===================== 3. 移除安装目录（仅编译产物，不碰源码） =====================
echo Removing Coly installation directory...
if exist "%COLY_ROOT%" (
    rmdir /s /q "%COLY_ROOT%"
    echo Removed directory: %COLY_ROOT%
) else (
    echo Directory not found: %COLY_ROOT% (skipping)
)

:: ===================== 最终验证 =====================
echo.
echo ========================================
echo Coly v1.5.3 Uninstallation Complete
echo ========================================
echo Removed: %COLY_ROOT% (installation directory)
echo Your source code (e.g., %cd%\Coly.cpp) is NOT touched!
echo Compilers (cl/g++) and MinGW/MSVC were not uninstalled.
echo ========================================

pause