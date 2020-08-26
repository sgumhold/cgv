@echo off
setlocal EnableDelayedExpansion

echo This script will download and extract the prebuilt binaries from the pytorch website.
echo Approximately 500MB of hard drive space is needed.
:choice
set /P c=Are you sure you want to continue[y/n]?
if /I "%c%" EQU "y" goto :run
if /I "%c%" EQU "n" goto :skip
goto :choice

rem Only execute the script if both the CGV directory and the CGV build directory are specified.

:run
if NOT "%CGV_DIR%" == "" if NOT "%CGV_BUILD%" == "" (
  set script=%CGV_DIR%\3rd\libtorch\get_prebuilt.ps1
  echo Starting script !script! to download libtorch...
  powershell -ExecutionPolicy ByPass -command ". '!script!'
  echo Donwload and extraction complete.

  set src=%CGV_DIR%\3rd\libtorch\dist\libtorch\lib
  set dst=%CGV_BUILD%\bin
  echo Copying required libtorch Dlls to !dst!...
  robocopy "!src!" "!dst!" asmjit.dll c10.dll fbgemm.dll libiomp5md.dll torch_cpu.dll /R:3 /W:5 /IS /IT

  echo You can now include "libtorch" in your projects dependencies.
) else (
  echo CGV_DIR and/or CGV_BUILD is not specified. Run define_system_variables first.
)

:skip
pause
