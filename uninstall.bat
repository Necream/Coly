@echo off
:: Coly v1.5.3 Uninstall Script (Windows)
:: Fully removes all Coly files and directories
:: Usage: Run as Administrator

setlocal enabledelayedexpansion

:: Define core paths (matches install script)
set COLY_ROOT=C:\Coly
set SYSTEM_BAT=C:\Windows\System32\coly.bat
set START_SCRIPT=%COLY_ROOT%\start-service.bat

:: Stop running services first
echo Stopping Coly services...
taskkill /f /im server.exe > nul 2>&1
taskkill /f /im coly.exe > nul 2>&1

:: Remove system-wide batch file
echo Removing system-wide access script...
if exist "%SYSTEM_BAT%" (
    del /f /q "%SYSTEM_BAT%"
    echo Removed: %SYSTEM_BAT%
) else (
    echo File not found: %SYSTEM_BAT% (skipping)
)

:: Remove main installation directory
echo Removing Coly installation directory...
if exist "%COLY_ROOT%" (
    rmdir /s /q "%COLY_ROOT%"
    echo Removed directory: %COLY_ROOT%
) else (
    echo Directory not found: %COLY_ROOT% (skipping)
)

:: Final verification
echo ========================================
echo Coly v1.5.3 Uninstallation Complete
echo ========================================
echo All Coly files and directories have been removed.
echo Note: MinGW-w64 (g++) was not uninstalled.
echo ========================================

pause