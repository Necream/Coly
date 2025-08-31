@echo off
echo Start to  install Coly...
md C:\Coly\Settings
md C:\Coly\TempCode
copy Coly.exe C:\Coly
copy Settings\LanguageMap.json C:\Coly\Settings
copy InteractiveColy.cly C:\Coly
echo Install finished.
pause