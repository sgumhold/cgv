@echo off
rem first make directory aktive where script is located
%~d0
cd %~p0
if exist openal-soft\core\ (
	echo submodule openal-soft source path core exists
) else (
	git submodule init openal-soft
	for /F "usebackq" %%v in (`git config -f ..\..\.gitmodules submodule.3rd/openal-soft/openal-soft.url`) do git clone --no-checkout --depth=1 %%v openal-soft
	cd openal-soft
	git sparse-checkout set --cone common core al alc include
	cd ..
	git submodule absorbgitdirs openal-soft
	git submodule update --init --recursive --force openal-soft
	echo sparsely checked out openal-soft
)
