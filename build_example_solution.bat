@echo off
rem set working directory to script directory
%~d0
cd %~p0
call bin\generate_makefiles.bat plugins\examples\examples.pj
