@echo off
setlocal EnableDelayedExpansion
%~d0
cd %~p0
echo Define System Platform for cgv Framework
echo -------------------------------------------
echo    please answer the following question by
echo    entering a number between 1 and 2 and
echo    pressing return afterwards.
echo -------------------------------------------
echo choose system platform:
echo.
echo interactive mode:
echo [1] ... WIN32 
echo [2] ... x64
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
   echo script canceled
   pause
   goto:eof
)
set cgvplatform=WIN32
if "%selection%" == "2" (
   call set cgvplatform=x64
)

call reg ADD HKEY_CURRENT_USER\Environment /v CGV_PLATFORM /t REG_SZ /d "%cgvplatform%" /f > nul 2> nul
bin\setx CGV_DUMMY ""
call reg DELETE HKEY_CURRENT_USER\Environment /v CGV_DUMMY /f  >nul 2> nul
echo set CGV_PLATTFORM to %cgvplatform%
pause
