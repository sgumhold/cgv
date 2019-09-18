@echo off
setlocal EnableDelayedExpansion
%~d0
cd %~p0
:menu
echo Define cgv options
echo -------------------------------------------
echo CGV_OPTIONS=%CGV_OPTIONS%
echo -------------------------------------------
echo [1] ... add NO_OPENVR
echo [2] ... clear all options
echo.
echo [q] ... quit script
echo.
:ask_again
set /P selection=choose 1-2 or q^>
if [%selection%] == [] (
   echo please enter a number between 1 and 2 or q for quit
   goto:ask_again
)
if "%selection%" == "q" (
   goto:eof
)
if "%selection%" == "1" (
	call :add_option NO_OPENVR
	goto :menu

)

call reg DELETE HKEY_CURRENT_USER\Environment /v CGV_OPTIONS /f  >nul 2> nul
bin\setx CGV_DUMMY ""
call reg DELETE HKEY_CURRENT_USER\Environment /v CGV_DUMMY /f  >nul 2> nul
set CGV_OPTIONS=
goto :menu

:add_option
call reg ADD HKEY_CURRENT_USER\Environment /v CGV_OPTIONS /t REG_SZ /d "%1%" /f > nul 2> nul
bin\setx CGV_DUMMY ""
call reg DELETE HKEY_CURRENT_USER\Environment /v CGV_DUMMY /f  >nul 2> nul
set CGV_OPTIONS=%1
goto :eof
