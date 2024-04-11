
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")
file(GLOB_RECURSE HEADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.h")

cgv_create_lib(cgv_algorithm SOURCES ${SOURCES} HEADERS ${HEADERS})
	
target_compile_definitions(cgv_algorithm PRIVATE CGV_ALGORITHM_EXPORTS)
target_compile_definitions(cgv_algorithm_static PRIVATE CGV_ALGORITHM_FORCE_STATIC)

# enforce '17 standard compliance
target_compile_features(cgv_algorithm PUBLIC cxx_std_17)
target_compile_features(cgv_algorithm_static PUBLIC cxx_std_17)
set_target_properties(cgv_algorithm cgv_algorithm_static PROPERTIES CXX_STANDARD_REQUIRED ON)
set_target_properties(cgv_algorithm cgv_algorithm_static PROPERTIES CXX_EXTENSIONS OFF)
