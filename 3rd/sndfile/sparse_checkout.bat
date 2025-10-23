@echo off
rem first make directory aktive where script is located
%~d0
cd %~p0
if exist libsndfile\src\ (
	echo submodule libsndfile source path src exists
) else (
	git submodule init libsndfile
	for /F "usebackq" %%v in (`git config -f ..\..\.gitmodules submodule.3rd/sndfile/libsndfile.url`) do git clone --no-checkout --depth=1 %%v libsndfile
	cd libsndfile
	git sparse-checkout set --cone src include 
	cd ..
	git submodule absorbgitdirs libsndfile
	git submodule update --init --recursive --force libsndfile
	echo sparsely checked out libsndfile
)
if exist vorbis\lib\ (
	echo submodule vorbis source path lib exists
) else (
	git submodule init vorbis
	for /F "usebackq" %%v in (`git config -f ..\..\.gitmodules submodule.3rd/sndfile/vorbis.url`) do git clone --no-checkout --depth=1 %%v vorbis
	cd vorbis
	git sparse-checkout set --cone lib include 
	cd ..
	git submodule absorbgitdirs vorbis
	git submodule update --init --recursive --force vorbis
	echo sparsely checked out vorbis
)
if exist ogg\src\ (
	echo submodule ogg source path src exists
) else (
	git submodule init ogg
	for /F "usebackq" %%v in (`git config -f ..\..\.gitmodules submodule.3rd/sndfile/ogg.url`) do git clone --no-checkout --depth=1 %%v ogg
	cd ogg
	git sparse-checkout set --cone src include 
	cd ..
	git submodule absorbgitdirs ogg
	git submodule update --init --recursive --force ogg
	echo sparsely checked out ogg
)
if exist flac\include\ (
	echo submodule flac source path include exists
) else (
	git submodule init flac
	for /F "usebackq" %%v in (`git config -f ..\..\.gitmodules submodule.3rd/sndfile/flac.url`) do git clone --no-checkout --depth=1 %%v flac
	cd flac
	git sparse-checkout set --cone include 
	cd ..
	git submodule absorbgitdirs flac
	git submodule update --init --recursive --force flac
	echo sparsely checked out flac
)
if exist opus\include\ (
	echo submodule opus source path include exists
) else (
	git submodule init opus
	for /F "usebackq" %%v in (`git config -f ..\..\.gitmodules submodule.3rd/sndfile/opus.url`) do git clone --no-checkout --depth=1 %%v opus
	cd opus
	git sparse-checkout set --cone include 
	cd ..
	git submodule absorbgitdirs opus
	git submodule update --init --recursive --force opus
	echo sparsely checked out opus
)