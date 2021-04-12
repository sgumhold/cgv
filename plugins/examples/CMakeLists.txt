
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")
file(GLOB_RECURSE SHADER_SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.gl*")
file(GLOB_RECURSE IMAGES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.png")

cgv_prepare_resources(${CMAKE_SOURCE_DIR}
	IMG_SOURCES
	IMG_INCLUDES
	${IMAGES}
)

add_library(examples
	${SOURCES}
	${IMG_SOURCES}
	${SHADER_SOURCES}
	${IMAGES}
)
target_link_libraries(examples
	delaunay
	plot
	cg_gamepad
	cgv_gl
	crg_grid
	crg_stereo_view
	cgv_render
	cgv_gui
	cgv_base
	cgv_type
	cgv_reflect
	cgv_utils
	cgv_signal
	cgv_data
	cgv_media
)
target_include_directories(examples PRIVATE
	../../libs 
	../../libs/delaunay 
)
add_dependencies(examples cgv_viewer)

# cgv_test_shaders(examples ${SHADER_SOURCES})
