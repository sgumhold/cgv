
# The CGV framework is needed
# find_package(cgv COMPONENTS utils type reflect data base media os gui render)

# find_package(cgv_gl)

set(CMAKE_CXX_FLAGS "-std=c++11")

# Source files
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")
# Header files
file(GLOB_RECURSE HEADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.h")
# Shader files
file(GLOB_RECURSE SHADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "glsl/*.gl*")

add_library(plot ${SOURCES} ${SHADERS})
target_include_directories(plot PUBLIC ${CGV_DIR}/libs)
target_link_libraries(plot
	cgv_utils
	cgv_type
	cgv_reflect
	cgv_data
	cgv_base
	cgv_media
	cgv_os
	cgv_gui
	cgv_render
)

# FIXME: For unicode support the definitions _UNICODE and UNICODE have to be
# set. Can we do this better?
# cgv_add_library(plot 
# 	SOURCES ${SOURCES} 
# 	PUBLIC_HEADERS ${HEADERS}
# 	SHADERS ${SHADERS}
# 	DEFINITIONS UNICODE _UNICODE
# 	SHARED_DEFINITIONS PLOT_EXPORTS
# 	STATIC_DEFINITIONS PLOT_FORCE_STATIC)
