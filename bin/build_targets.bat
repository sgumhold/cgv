@echo off
SETLOCAL ENABLEDELAYEDEXPANSION
set i=1
FOR %%A IN (%*) DO (
	set param=%%A
	set second=!param:*.=!
	call set first=%%param:.!second!=%%
	set config="Debug Dll"
	if "!second!"=="D" set config="Debug"
	if "!second!"=="DD" set config="Debug Dll"
	if "!second!"=="DE" set config="Debug Exe"
	if "!second!"=="R"  set config="Release"
	if "!second!"=="RD" set config="Release Dll"
	if "!second!"=="RE" set config="Release Exe"
	if exist !first!\!first!.sln (
		start "Build !param!" /D!first! /LOW /SEPARATE cmd /c "devenv !first!.sln /Build !config! & pause"
		echo started !first!:!config!
	) else (
		echo WARNING: solution !first!\!first!.sln does not exist!!!
	)
	set /a i+=1
)
pause
