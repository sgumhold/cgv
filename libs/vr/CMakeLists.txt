
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")

cgv_create_lib(vr SOURCES ${SOURCES} DEPENDENCIES glew)

target_compile_definitions(vr PRIVATE VR_KIT_EXPORTS)
