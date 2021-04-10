
# Source files
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")
# Header files
file(GLOB_RECURSE PUBLIC_HEADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.h")
file(GLOB_RECURSE PUBLIC_HEADERS_HH RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.hh")

add_library(cgv_media ${SOURCES})
target_include_directories(cgv_media PUBLIC
	${CGV_DIR}
)
target_link_libraries(cgv_media PUBLIC
	cgv_utils
	cgv_type
	cgv_data
	cgv_base
)

if (UNIX)
	target_compile_options(cgv_media PRIVATE -fpermissive)
endif()

# # This component has multiple static and shared definitions
# cgv_add_export_definitions(cgv_media CGV_MEDIA_FONT CGV_MEDIA_ILLUM CGV_MEDIA_IMAGE CGV_MEDIA_VIDEO)

