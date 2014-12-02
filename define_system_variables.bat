@echo off
setlocal EnableDelayedExpansion
%~d0
cd %~p0
set my_path=%~dp0
set cgvdir=%my_path:~0,-1%
set cgvbuild=%cgvdir%\build
set cgvinstall=%cgvdir%
if [%1] == [] (
   set param=""
) else (
   set param=%1
)
set param=%param:"=%
if NOT [%1] == [] (
   if EXIST "%param%\..\%~n1" (
      call set cgvbuild=%param%
      call set cgvinstall=%param%
   ) else ( 
      echo only directories valid as build path argument
      echo "%param%\..\%~n1" does not exist
   )
)
echo Define System Variables for cgv Framework
echo -------------------------------------------
echo    please answer the following question by
echo    entering a number between 1 and 6 and
echo    pressing return afterwards.
echo -------------------------------------------
echo choose whether to proceed interactively and
echo which compiler to choose:
echo.
echo interactive mode:
echo [1] ... Microsoft Visual Studio 2005 
echo [2] ... Microsoft Visual Studio 2008 
echo [3] ... Microsoft Visual Studio 2008 Express
echo [4] ... Microsoft Visual Studio 2010
echo [5] ... Microsoft Visual Studio 2012
echo [6] ... Microsoft Visual Studio 2013
echo.
echo automatic mode:
echo [7] ... Microsoft Visual Studio 2005 
echo [8] ... Microsoft Visual Studio 2008 
echo [9] ... Microsoft Visual Studio 2008 Express
echo [a] ... Microsoft Visual Studio 2010
echo [b] ... Microsoft Visual Studio 2012
echo [c] ... Microsoft Visual Studio 2013
echo.
echo [q] ... quit script
echo.
:ask_again
set /P selection=choose 1-9 or a-c or q^>
if [%selection%] == [] (
   echo please enter a number between 1 and 9 or a or q for quit
   goto:ask_again
)
if "%selection%" == "q" (
   echo script canceled
   pause
   goto:eof
)
set valid_number=true
if "%selection%" == "a" (
   call set /A selection=10 
) else (
   if "%selection%" == "b" (
      call set /A selection=11 
   ) else (
      if "%selection%" == "c" (
         call set /A selection=12
      ) else (
	call set /A selection=%selection%
	if %selection% LSS 1 (set valid_number=false)
	if %selection% GTR 9 (set valid_number=false)
      )
   )
)
if %valid_number% == false (
   echo invalid number outside range [1-9,a-c], please try again
   goto:ask_again
)
set interactive_mode=true
if %selection% GTR 6 (
   set interactive_mode=false
   set /A selection=selection-6
)
set cgvcompiler=undefined
call set condition=%selection%==1
if %condition% (set cgvcompiler=vs8)
call set condition=%selection%==2
if %condition% (set cgvcompiler=vs9)
call set condition=%selection%==3
if %condition% (set cgvcompiler=vs9)
call set condition=%selection%==4
if %condition% (set cgvcompiler=vs10)
call set condition=%selection%==5
if %condition% (set cgvcompiler=vs11)
call set condition=%selection%==6
if %condition% (set cgvcompiler=vs12)
if %interactive_mode%==true (
   echo.
   echo selected interactive mode
   echo.
)
call :set_variable CGV_DIR "%cgvdir%" %interactive_mode%
call :set_variable CGV_BUILD "%cgvbuild%" %interactive_mode%
call :set_variable CGV_INSTALL "%cgvinstall%" %interactive_mode%
call :set_variable CGV_COMPILER "%cgvcompiler%" %interactive_mode%


rem find out current value of path variable
call :query_user_path old_path
set target_path=%%CGV_INSTALL%%\bin
call :remove_from_list old_path target_path

set old_path_1=%new_path%
set target_path=%%CGV_DIR%%\bin
call :remove_from_list old_path_1 target_path
if "%new_path%" == "" (
  set new_path=%%CGV_DIR%%\bin;%%CGV_INSTALL%%\bin
) else (
  set new_path=%%CGV_DIR%%\bin;%%CGV_INSTALL%%\bin;%new_path%
)
set change=false
if "%new_path%" NEQ "%old_path%" (
   set change=true
)
if %change%==true (
   echo.
   echo old Path extension of USER:
   echo ---------------------------
   call :show_list_expanded old_path
   echo.
   echo new Path extension of USER:
   echo ---------------------------
   call :show_list_expanded new_path
)
set allow_change=y
:yes_no_main
if %change%==true (
   if %interactive_mode%==true (
      echo.
      set /p allow_change=allow path change [y/n] ^>
   )
)
if "%allow_change%" == "n" (
    set change=false
) else (
   if %allow_change% NEQ y (
      echo.
      echo please answer y, n
      goto:yes_no
   )
)
if %change%==true (
   reg ADD HKEY_CURRENT_USER\Environment /v Path /t REG_EXPAND_SZ /d "%new_path%" /f > nul 2> nul
)
bin\setx CGV_DUMMY ""
call reg DELETE HKEY_CURRENT_USER\Environment /v CGV_DUMMY /f  >nul 2> nul
pause
goto:eof

:set_variable
SETLOCAL
set var_name=%1
set new_value=%2
set new_value=%new_value:~1,-1%
set interactive_mode=%3
rem echo called set_variable(%var_name%,%new_value%,%interactive_mode%)
if defined %var_name% (
   call set old_value=%%%var_name%%%
)
set change=true
if defined %var_name% (
   if "%old_value%" NEQ "%new_value%" (
      echo %var_name%: %old_value% -^> %new_value%
   ) else (
      echo %var_name%: no change
      set change=false
   )
) else (
   echo %var_name%: set to %new_value%
)
set allow_change=y
:yes_no
if %change%==true (
   if %interactive_mode%==true (
      echo.
      set /p allow_change=allow change [y/n] ^>
   )
)
if %allow_change% == n (
    set change=false
) else (
   if %allow_change% NEQ y (
      echo.
      echo please answer y or n
      goto:yes_no
   )
)
if %change%==true (
   call reg ADD HKEY_CURRENT_USER\Environment /v %var_name% /t REG_SZ /d "%new_value%" /f > nul 2> nul
   endlocal & call set %var_name%=%new_value%
   echo       ^(USER:%var_name%=%new_value%^)
)
echo.
goto:eof


:show_list_expanded
setlocal
call set list=%%%1%%
if "%list%" == "[]" (
  echo empty
  goto :eof
)
for %%G in ("%list:;=" "%") do @echo %%G
goto:eof

:remove_from_list
setlocal
call set list=%%%1%%
if "%list%" == "[]" (
  goto :eof
)
if "%list%" == "" (
  goto :eof
)
call set element=%%%2%%
for %%G in ("%list:;=" "%") do (
   set entry=%%G
   set entry=!entry:~1,-1!
   if !entry! NEQ %element% (
		if defined new_list (
		   set new_list=!new_list!;!entry!
		) else (
		   set new_list=!entry!
		)
   )
)
endlocal & set new_path=%new_list%
endlocal
goto:eof


:query_user_path
reg QUERY HKEY_CURRENT_USER\Environment /v Path > nul 2> nul
if ERRORLEVEL 1 (
   set %1=[]
) else (
   call :query_user_path_impl %1
)
goto:eof

:query_user_path_impl
for /f "delims=" %%G IN ('reg QUERY HKEY_CURRENT_USER\Environment /v Path ^|find "Path"') DO (set user_path=%%G)
call :remove_spaces user_path
call :remove_non_spaces user_path
call :remove_spaces user_path
call :remove_non_spaces user_path
call :remove_spaces user_path
set %1=%user_path%
goto :eof

:remove_spaces
call set temp=%%%1%%
:loop_remove_spaces
if "%temp:~0,1%" == " " (
   set temp=%temp:~1%
   goto :loop_remove_spaces
)
if "%temp:~0,1%" == "	" (
   set temp=%temp:~1%
   goto :loop_remove_spaces
)
set %1=%temp%
goto:eof

:remove_non_spaces
call set temp=%%%1%%
:loop_remove_non_spaces
if "%temp:~0,1%" == " " (goto :end_remove_non_spaces)
if "%temp:~0,1%" == "	" (goto :end_remove_non_spaces)
set temp=%temp:~1%
goto :loop_remove_non_spaces
:end_remove_non_spaces
set %1=%temp%
goto:eof