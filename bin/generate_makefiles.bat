@echo off
if not defined CGV_COMPILER (
   echo system variable CGV_COMPILER must be defined
   echo please use the define_systems_variable.bat in the cgv root dir
   goto:eof
)
%~d1
cd %~p1
call "%CGV_DIR%\bin\ppp" -script "%CGV_DIR%\make\ppp\templates\make.ppp" "%~f1"
echo to open solution, press any key (otherwise close window)
pause
"%CGV_BUILD%\%CGV_COMPILER%\%~n1\%~n1.sln"
