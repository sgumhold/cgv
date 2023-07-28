@echo off
setlocal EnableDelayedExpansion
%~d0
cd %~p0
:menu
echo Define cgv options
echo -------------------------------------------
echo CGV_OPTIONS=%CGV_OPTIONS%
echo -------------------------------------------
echo [1] ... toggle NO_OPENVR (allows to avoid the start of openvr during development)
echo [2] ... toggle SHADER_DEVELOPER (make shader_test throw errors that cause the build process to fail)
echo [3] ... toggle ENCODE_SHADER_BASE64 (encode shader codes in base64 before embedding)
echo [4] ... toggle STDCPP17 (later makes cgv::utils use std::filesystem)
echo [5] ... toggle BUILD_WITH_AUDIO (requires checking out the git submodules when building from the repository)
echo [6] ... toggle OPENMP (uses OpenMP as default for all projects)
echo [7] ... toggle custom option
echo [8] ... clear all options
echo.
echo [q] ... quit script
echo.
:ask_again
set /P selection=choose 1-8 or q^>
if [%selection%] == [] (
   echo please enter a number between 1 and 8 or q for quit
   goto:ask_again
)
if "%selection%" == "q" (
   goto:eof
)
if "%selection%" == "1" (
	call :toggle_in_list CGV_OPTIONS NO_OPENVR
	goto :publish_options
)
if "%selection%" == "2" (
	call :toggle_in_list CGV_OPTIONS SHADER_DEVELOPER
	goto :publish_options
)
if "%selection%" == "3" (
	call :toggle_in_list CGV_OPTIONS ENCODE_SHADER_BASE64
	goto :publish_options
)
if "%selection%" == "4" (
	call :toggle_in_list CGV_OPTIONS STDCPP17
	goto :publish_options
)
if "%selection%" == "5" (
	call :toggle_in_list CGV_OPTIONS BUILD_WITH_AUDIO
	goto :publish_options
)
if "%selection%" == "6" (
	call :toggle_in_list CGV_OPTIONS OPENMP
	goto :publish_options
)
if "%selection%" == "7" (
	set /P custom_option=enter curstom option:^>
	call :toggle_in_list CGV_OPTIONS !custom_option!
	goto :publish_options
)
rem clear all options by removal of registry key and using setx to communicate change to all processes
call reg DELETE HKEY_CURRENT_USER\Environment /v CGV_OPTIONS /f  >nul 2> nul
bin\setx CGV_DUMMY ""
call reg DELETE HKEY_CURRENT_USER\Environment /v CGV_DUMMY /f  >nul 2> nul
set CGV_OPTIONS=
goto :menu

:publish_options
call reg ADD HKEY_CURRENT_USER\Environment /v CGV_OPTIONS /t REG_SZ /d "%CGV_OPTIONS%" /f > nul 2> nul
bin\setx CGV_DUMMY ""
call reg DELETE HKEY_CURRENT_USER\Environment /v CGV_DUMMY /f  >nul 2> nul
goto :menu


rem call with two arguments: variable name and to be toggled element
rem the variable is assumed to contain a semicolon separated list of
rem elements. If the passed element is in the list, it is removed and
rem the variable will contain the truncated list afterwards. If the 
rem element is not in the list, it is appended to the list. 
rem The list is allowed to be empty and optionally can be enclosed in 
rem in double quotes. The quoting property is preserved by the function.
rem To handle quotes it is assumed that the # character is not used inside 
rem of elements.
:toggle_in_list
setlocal EnableDelayedExpansion
rem extract parameters
set varname=%1
set element=%2
rem extract list into local variable
call set list=%%%1%%
rem replace potential double quotes with the # 
if defined list (
	set list=!list:"=#!
)
rem in case of empty list, result is just the element with quoting property of previously empty list 
if "!list!" == "" (
	endlocal & set %1=%element%
	goto :eof
) else (
	rem this would yield an error if we did not replace double quotes with # before
	if "!list!" == "##" (
		endlocal & set %1="%element%"
		goto :eof
	)
)
rem determine quoting proterty into local variable
if "!list:~0,1!" == "#" (
	set has_quotes=true
)
rem remove quotes, such that we can assume that the list is unquoted
set list=!list:#=!
rem split list into quoted elements
set list="%list:;=" "%"
rem flag to let us know whether we are at first element and do not need to add a semicolon 
set is_first=1
rem remember whether we removed to element during iteration of the list to know whether we need to append it
set removed=0
rem iterate all list elements
for %%G in (%list%) do (
	rem check for to be toggled element and do not append it to new list, but set removed status
	if %%G == "%element%" (
		set removed=1
	) else (
		rem otherwise prepend a semicolon starting with the second element
		set temp=%%G
		if !is_first! == 1 (
			set is_first=0
		) else (
			set new_list=!new_list!;
		)
		rem and the append the current element to the new list
		set new_list=!new_list!!temp:~1,-1!
	)
)
rem if to be toggled element is not found, append it to new list 
if %removed% == 0 (
	  if !is_first! == 1 (
		set is_first=0
	  ) else (
	  	  set new_list=!new_list!;
	  )
  	  set new_list=!new_list!!element!
)
rem preserve original quoting property
if "%has_quotes%" == "true" (
	set new_list="!new_list!"
)
rem pass the new list to the variable of the calling shell instance
endlocal & set %varname%=%new_list%
goto:eof
