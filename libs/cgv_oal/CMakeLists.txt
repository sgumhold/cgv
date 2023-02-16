
cgv_add_target(cgv_oal
	TYPE library
	SOURCES
		lib_begin.h
		al_context.h
		al_context.cpp
	DEPENDENCIES
		OpenAL::OpenAL SndFile::sndfile cgv_math
)

# enforce '17 standard compliance
target_compile_features(cgv_oal PUBLIC cxx_std_17)
target_compile_features(cgv_oal_static PUBLIC cxx_std_17)
set_target_properties(cgv_oal cgv_oal_static PROPERTIES CXX_STANDARD_REQUIRED ON)
set_target_properties(cgv_oal cgv_oal_static PROPERTIES CXX_EXTENSIONS OFF)
