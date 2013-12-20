@echo off

echo This script changes file association of .scn and .scene files 
echo to the script scene_graph.bat, which need to be located in the 
echo same directory as this script.
echo.
echo current association for .pj :
assoc .pj
echo.
echo current file type for CGV.Makefile
echo.
FTYPE CGV.Makefile
echo.
goto check_Permissions

:continue_script
echo.
echo If you do not want to associate pj extension with file type
echo CGV.Makefile, press Ctrl-c now, otherwise ENTER!
pause > nul
assoc .pj=CGV.Makefile
ftype CGV.Makefile=%~dp0bin\generate_makefiles.bat %%1 %%2 %%3 %%4
goto exit

:check_Permissions
    echo Administrative permissions required to change file associations. 
    echo Detecting permissions...

    net session >nul 2>&1
    if %errorLevel% == 0 (
        echo Success: Administrative permissions confirmed.
	goto continue_script
    ) else (
        echo Failure: Current permissions inadequate. 
	echo.
	echo Please start script again with admin privileges.
    )
:exit
    pause