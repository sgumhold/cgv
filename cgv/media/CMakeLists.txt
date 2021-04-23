
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")

add_library(cgv_media ${SOURCES})
target_compile_definitions(cgv_media PRIVATE
        CGV_MEDIA_EXPORTS
        CGV_MEDIA_FONT_EXPORTS
        CGV_MEDIA_ILLUM_EXPORTS
        CGV_MEDIA_IMAGE_EXPORTS
        CGV_MEDIA_VIDEO_EXPORTS)
target_link_libraries(cgv_media PUBLIC
        cgv_utils
        cgv_type
        cgv_data
        cgv_base)

if (UNIX)
    target_compile_options(cgv_media PRIVATE -fpermissive)
endif ()

target_include_directories(cgv_media PUBLIC
        $<BUILD_INTERFACE:${CGV_DIR}>
        $<INSTALL_INTERFACE:include>)

install(TARGETS cgv_media EXPORT cgv DESTINATION ${CGV_LIB_DEST})
