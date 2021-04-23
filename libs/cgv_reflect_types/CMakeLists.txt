
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")

add_library(cgv_reflect_types ${SOURCES})
target_link_libraries(cgv_reflect_types
        cgv_utils
        cgv_type
        cgv_reflect
        cgv_media
        cgv_math
        cgv_render
        cgv_gui)

target_include_directories(cgv_reflect_types PUBLIC
        $<BUILD_INTERFACE:${CGV_DIR}>
        $<BUILD_INTERFACE:${CGV_DIR}/libs>
        $<INSTALL_INTERFACE:${CGV_LIBS_INCLUDE_DEST}>)

install(TARGETS cgv_reflect_types EXPORT cgv_reflect_types DESTINATION ${CGV_LIB_DEST})
install(EXPORT cgv_reflect_types DESTINATION ${CGV_LIB_DEST})
