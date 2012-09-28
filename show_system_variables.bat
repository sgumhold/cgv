@echo off
call :query_user_path old_path
echo current Path extension of USER:
echo -------------------------------
call :show_list_expanded old_path
echo.
echo CGV_DIR:      %CGV_DIR%
echo CGV_BUILD:    %CGV_BUILD%
echo CGV_INSTALL:  %CGV_INSTALL%
echo CGV_COMPILER: %CGV_COMPILER%
echo CGV_PROJECT_DIR: %CGV_PROJECT_DIR%
echo CGV_SUPPORT_DIR: %CGV_SUPPORT_DIR%
pause
goto:eof

:show_list_expanded
setlocal
call set list=%%%1%%
if "%list%" == "[]" (
  echo empty
) else (
  for %%G in ("%list:;=" "%") do @echo %%G
)
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