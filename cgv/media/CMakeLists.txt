
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")

cgv_create_lib(cgv_media CORE_LIB SOURCES ${SOURCES}
        DEPENDENCIES cgv_utils cgv_type cgv_data cgv_base cgv_os)

target_compile_definitions(cgv_media PRIVATE
        CGV_MEDIA_FONT_EXPORTS
        CGV_MEDIA_ILLUM_EXPORTS
        CGV_MEDIA_IMAGE_EXPORTS
        CGV_MEDIA_VIDEO_EXPORTS)

if (UNIX)
    target_compile_options(cgv_media PRIVATE -fpermissive)
    target_compile_options(cgv_media_static PRIVATE -fpermissive)
endif ()
