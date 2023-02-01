
#cgv_create_lib(cgv_oal
#    SOURCES
#        lib_begin.h
#        al_context.h
#        al_context.cpp
#    DEPENDENCIES
#        cgv_math
#)

cgv_add_target(cgv_oal
	TYPE library
	SOURCES
		lib_begin.h
		al_context.h
		al_context.cpp
    DEPENDENCIES
		cgv_math
)
# TODO: investigate how to best integrate linking to these imported libs using the CGV CMake pipeline
target_link_libraries(cgv_oal PUBLIC OpenAL::OpenAL SndFile::sndfile)
target_link_libraries(cgv_oal_static PUBLIC OpenAL::OpenAL SndFile::sndfile)

# enforce '17 standard compliance
target_compile_features(cgv_oal PUBLIC cxx_std_17)
target_compile_features(cgv_oal_static PUBLIC cxx_std_17)
set_target_properties(cgv_oal PROPERTIES CXX_STANDARD_REQUIRED ON)
set_target_properties(cgv_oal_static PROPERTIES CXX_STANDARD_REQUIRED ON)
set_target_properties(cgv_oal PROPERTIES CXX_EXTENSIONS OFF)
set_target_properties(cgv_oal_static PROPERTIES CXX_EXTENSIONS OFF)
