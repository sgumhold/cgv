
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")

add_library(vr ${SOURCES})
target_compile_definitions(vr PRIVATE VR_KIT_EXPORTS)
target_link_libraries(vr glew)
target_include_directories(vr PUBLIC
        $<BUILD_INTERFACE:${CGV_DIR}>
        $<BUILD_INTERFACE:${CGV_DIR}/libs>
        $<INSTALL_INTERFACE:${CGV_LIBS_INCLUDE_DEST}>)

install(TARGETS vr EXPORT cgv_libs DESTINATION ${CGV_LIB_DEST})
