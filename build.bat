@echo off
echo You need G++/MSVC to build this project.
echo If you have it installed, please run the appropriate build script:
:Choice
set /p Choice=Are you want to build the latest version? (Y/N)
if /I "%Choice%"=="Y" goto build_latest
if /I "%Choice%"=="N" goto build_now
echo Invalid choice. Please enter Y or N.:
goto Choice
:build_latest
git pull
md temp
cd temp
git clone https://github.com/Necream/GXPass.git
cd GXPass
copy GXPass ..\..
cd ..\..
rd /s /q temp
:build_now
set /p Choice=Do you want to build with G++ (MinGW) or MSVC? (G/M)
if /I "%Choice%"=="G" goto build_gpp
if /I "%Choice%"=="M" goto build_msvc
echo Invalid choice. Please enter G or M.:
goto build_now
:build_gpp
g++ Coly.cpp -o Coly.exe -I.
echo Build complete. You can run Coly.exe now.
goto end
:build_msvc
cl /EHsc Coly.cpp
echo Build complete. You can run Coly.exe now.
goto end
:end
pause