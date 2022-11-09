rem @echo off
%~d0
cd %~p0
cd openal-soft\alc
pause
rename backends\alc_backends_null.cpp       null.cpp
rename effects\alc_effects_autowah.cpp     autowah.cpp 
rename effects\alc_effects_chorus.cpp      chorus.cpp
rename effects\alc_effects_compressor.cpp  compressor.cpp
rename effects\alc_effects_convolution.cpp convolution.cpp
rename effects\alc_effects_dedicated.cpp   dedicated.cpp
rename effects\alc_effects_distortion.cpp  distortion.cpp
rename effects\alc_effects_echo.cpp        echo.cpp
rename effects\alc_effects_equalizer.cpp   equalizer.cpp
rename effects\alc_effects_fshifter.cpp    fshifter.cpp
rename effects\alc_effects_modulator.cpp   modulator.cpp
rename effects\alc_effects_null.cpp        null.cpp
rename effects\alc_effects_pshifter.cpp    pshifter.cpp
rename effects\alc_effects_reverb.cpp      reverb.cpp
rename effects\alc_effects_vmorpher.cpp    vmorpher.cpp
rename alc_context.cpp             context.cpp
rename alc_device.cpp              device.cpp
pause