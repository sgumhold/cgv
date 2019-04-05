cmake_minimum_required(VERSION 2.6)
project(cg_icons)

# The CGV framework is needed
find_package(cgv COMPONENTS base)
find_package(res_prep)

set(SOURCES main.cxx)

# Resource files
file(GLOB_RECURSE IMAGES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.png")

include_directories(${cgv_INCLUDE_DIRS})

cgv_prepare_resources(${CMAKE_SOURCE_DIR} IMG_SOURCES ${IMAGES})

cgv_add_module(cg_icons 
	SOURCES ${SOURCES} ${IMG_SOURCES}
	FILES ${IMAGES}
)

cgv_add_export_definitions(cg_icons ICON)

target_link_libraries(cg_icons ${cgv_LIBRARIES})

cgv_write_find_file(cg_icons)

