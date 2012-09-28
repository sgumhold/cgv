@echo off
%~d1
cd %~p1
call "%CGV_DIR%\bin\ppp" -CGV_COMPILER=vs9e -script "%CGV_DIR%\make\ppp\templates\make.ppp" "%~f1"
pause
