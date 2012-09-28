call "%CGV_DIR%\bin\ppp.exe" -CGV_COMPILER=vs8 "%~f1" "%CGV_DIR%\make\ppp\templates\vs_vcproj.tm" "%CGV_BUILD%\vs8\%~n1\%~n1.vcproj" "%CGV_DIR%\make\ppp\templates\vs_sln.tm" "%CGV_BUILD%\vs8\%~n1\%~n1.sln" "%CGV_DIR%\make\ppp\templates\vs_vcproj_user.tm" "%CGV_BUILD%\vs8\%~n1\%~n1.vcproj.%USERDOMAIN%.%USERNAME%.user"
"%CGV_BUILD%\vs8\%~n1\%~n1.sln"
pause
			