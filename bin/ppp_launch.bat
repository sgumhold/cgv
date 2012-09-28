@echo off
if not defined CGV_COMPILER (
   echo system variable CGV_COMPILER must be defined
   echo please use the define_systems_variable.bat in the cgv root dir
   goto:eof
)
call "%CGV_DIR%\bin\ppp.exe" "%~f1" "%CGV_DIR%\make\ppp\templates\vs_vcproj.tm" "%CGV_BUILD%\%CGV_COMPILER%\%~n1\%~n1.vcproj" "%CGV_DIR%\make\ppp\templates\vs_sln.tm" "%CGV_BUILD%\%CGV_COMPILER%\%~n1\%~n1.sln" "%CGV_DIR%\make\ppp\templates\vs_vcproj_user.tm" "%CGV_BUILD%\%CGV_COMPILER%\%~n1\%~n1.vcproj.%USERDOMAIN%.%USERNAME%.user"
"%CGV_BUILD%\%CGV_COMPILER%\%~n1\%~n1.sln"
pause
			