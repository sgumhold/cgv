call "%CGV_DIR%\bin\ppp.exe" -CGV_COMPILER=gcc "%~f1" "%CGV_DIR%\make\ppp\templates\makefile.tm" "%CGV_BUILD%\gcc\%~n1\makefile"
call "%CGV_DIR%\bin\ppp.exe" -CGV_COMPILER=gcc -CGV_IDE=eclipse "%~f1" "%CGV_DIR%\make\ppp\templates\eclipse_project.tm" "%CGV_BUILD%\eclipse\%~n1\.project"
call "%CGV_DIR%\bin\ppp.exe" -CGV_COMPILER=gcc -CGV_IDE=eclipse "%~f1" "%CGV_DIR%\make\ppp\templates\eclipse_cproject.tm" "%CGV_BUILD%\eclipse\%~n1\.cproject"
