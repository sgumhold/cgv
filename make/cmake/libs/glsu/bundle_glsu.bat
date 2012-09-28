mkdir %CGV_BUILD%\bundles\glsu
mkdir %CGV_BUILD%\bundles\glsu\include\GL
mkdir %CGV_BUILD%\bundles\glsu\src
mkdir %CGV_BUILD%\bundles\glsu\test
mkdir %CGV_BUILD%\bundles\glsu\lib
mkdir %CGV_BUILD%\bundles\glsu\bin
copy %CGV_DIR%\libs\glsu\GL\*.h %CGV_BUILD%\bundles\glsu\include\GL
copy %CGV_DIR%\libs\glsu\*.c %CGV_BUILD%\bundles\glsu\src
copy %CGV_DIR%\libs\glsu\*.def %CGV_BUILD%\bundles\glsu\src
copy %CGV_DIR%\libs\glsu\*.txt %CGV_BUILD%\bundles\glsu
copy %CGV_DIR%\test\libs\glsu\*.h %CGV_BUILD%\bundles\glsu\test
copy %CGV_DIR%\test\libs\glsu\*.cxx %CGV_BUILD%\bundles\glsu\test
copy %~dp0CMakeLists.txt %CGV_BUILD%\bundles\glsu
copy %~dp0src\CMakeLists.txt %CGV_BUILD%\bundles\glsu\src
copy %~dp0test\CMakeLists.txt %CGV_BUILD%\bundles\glsu\test

pause