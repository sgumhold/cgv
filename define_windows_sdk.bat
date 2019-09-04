@echo off
setlocal ENABLEDELAYEDEXPANSION

if [%CGV_WINDOWS_SDK%]== [] (
	echo no current windows sdk version defined
) else (
	echo current windows sdk version: %CGV_WINDOWS_SDK%
)
echo -------------------------------
echo installed windows sdk versions:
set /a i=0
for /D %%g in ("%ProgramFiles(x86)%\Microsoft SDKs\Windows Kits\10\ExtensionSDKs\Microsoft.UniversalCRT.Debug\*") do (
	echo [!i!] ... %%~nxg
	set sdk_versions[!i!]=%%~nxg
	set last_i=!i!
	set last_sdk_version=%%~nxg
    set /a i = i+1 
)
echo -------------------------------
set /P selection=choose version [0-%last_i%] or press enter to select %last_sdk_version%:
if [%selection%] == [] (
	echo pressed enter
	call :set_variable "%last_sdk_version%"
) else (
	if [sdk_versions[%selection%]] == [] (
		goto :eof
	) else (
		echo !sdk_versions[%selection%]!
		call :set_variable "!sdk_versions[%selection%]!"
	)
)
endlocal
goto :eof

:set_variable
setlocal
set new_value=%1
set new_value=%new_value:~1,-1%
call reg ADD HKEY_CURRENT_USER\Environment /v CGV_WINDOWS_SDK /t REG_SZ /d "%new_value%" /f > nul 2> nul
bin\setx CGV_DUMMY ""
call reg DELETE HKEY_CURRENT_USER\Environment /v CGV_DUMMY /f  >nul 2> nul
endlocal & call set CGV_WINDOWS_SDK=%new_value%
echo ^(USER:CGV_WINDOWS_SDK=%CGV_WINDOWS_SDK%^)
pause