rem @echo off
%~d0
cd %~p0
cd openal-soft\alc
pause
rename backends\null.cpp       alc_backends_null.cpp
rename effects\autowah.cpp     alc_effects_autowah.cpp 
rename effects\chorus.cpp      alc_effects_chorus.cpp
rename effects\compressor.cpp  alc_effects_compressor.cpp
rename effects\convolution.cpp alc_effects_convolution.cpp
rename effects\dedicated.cpp   alc_effects_dedicated.cpp
rename effects\distortion.cpp  alc_effects_distortion.cpp
rename effects\echo.cpp        alc_effects_echo.cpp
rename effects\equalizer.cpp   alc_effects_equalizer.cpp
rename effects\fshifter.cpp    alc_effects_fshifter.cpp
rename effects\modulator.cpp   alc_effects_modulator.cpp
rename effects\null.cpp        alc_effects_null.cpp
rename effects\pshifter.cpp    alc_effects_pshifter.cpp
rename effects\reverb.cpp      alc_effects_reverb.cpp
rename effects\vmorpher.cpp    alc_effects_vmorpher.cpp
rename context.cpp             alc_context.cpp
rename device.cpp              alc_device.cpp
pause