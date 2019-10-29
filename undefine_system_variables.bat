@echo off
setlocal EnableDelayedExpansion
rem set working directory to script directory
%~d0
cd %~p0
rem find out current value of path variable
call :query_user_path old_path
set target_path=%%CGV_INSTALL%%\bin
call :remove_from_list old_path target_path

set old_path_1=%new_path%
set target_path=%%CGV_DIR%%\bin
call :remove_from_list old_path_1 target_path
echo old Path extension of USER:
echo ---------------------------
call :show_list_expanded old_path
echo.
echo new Path extension of USER:
echo ---------------------------
call :show_list_expanded new_path
if "%new_path%" == "" (
   echo deleting reg entry for PATH
   pause
   reg DELETE HKEY_CURRENT_USER\Environment /v Path /f > nul 2> nul
) else (
   reg ADD HKEY_CURRENT_USER\Environment /v Path /t REG_EXPAND_SZ /d "%new_path%" /f > nul 2> nul
)
reg DELETE HKEY_CURRENT_USER\Environment /v CGV_DIR /f >nul 2> nul
reg DELETE HKEY_CURRENT_USER\Environment /v CGV_BUILD /f  >nul 2> nul
reg DELETE HKEY_CURRENT_USER\Environment /v CGV_DATA /f  >nul 2> nul
reg DELETE HKEY_CURRENT_USER\Environment /v CGV_WINDOWS_SDK /f  >nul 2> nul
reg DELETE HKEY_CURRENT_USER\Environment /v CGV_PLATFORM /f  >nul 2> nul
reg DELETE HKEY_CURRENT_USER\Environment /v CGV_CHARSET /f  >nul 2> nul
reg DELETE HKEY_CURRENT_USER\Environment /v CGV_OPTIONS /f  >nul 2> nul
reg DELETE HKEY_CURRENT_USER\Environment /v CGV_INSTALL /f  >nul 2> nul
reg DELETE HKEY_CURRENT_USER\Environment /v CGV_COMPILER /f  >nul 2> nul
reg DELETE HKEY_CURRENT_USER\Environment /v CGV_PROJECT_DIR /f  >nul 2> nul
reg DELETE HKEY_CURRENT_USER\Environment /v CGV_SUPPORT_DIR /f  >nul 2> nul
bin\setx CGV_DUMMY ""
call reg DELETE HKEY_CURRENT_USER\Environment /v CGV_DUMMY /f  >nul 2> nul
echo.
echo undefined the variables CGV_DIR, CGV_BUILD, CGV_INSTALL, CGV_COMPILER, CGV_PLATFORM, CGV_WINDOWS_SDK, CGV_OPTIONS, CGV_DATA, CGV_PROJECT_DIR and CGV_SUPPORT_DIR
echo.
pause
goto:eof

:show_list_expanded
setlocal
call set list=%%%1%%
if "%list%" == "[]" (
  echo empty
  goto :eof
)
if "%list%" == "" (
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