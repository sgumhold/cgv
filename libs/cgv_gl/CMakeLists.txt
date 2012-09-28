cmake_minimum_required(VERSION 2.6)
project(cgv_gl)

# Include base definitions
include("../../cmake/buildCoreComponents.cmake")

# The CGV framework is needed
find_package(cgv COMPONENTS  utils type data base signal math media render)

# The PPP is needed
find_package(ppp)

# Find 3rd party dependencies
cgv_find_package(GLEW)
cgv_find_package(OpenGL REQUIRED)


set(HEADERS
	gl/gl.h
	gl/gl_context.h
	gl/gl_cursor.h
	gl/gl_depth_peeler.h
	gl/gl_image_drawable_base.h
	gl/gl_implicit_surface_drawable_base.h
	gl/gl_mesh_drawable_base.h
	gl/gl_tools.h
	gl/gl_transparent_renderer.h
	gl/gl_view.h
	gl/lib_begin.h)

set(SOURCES
	gl/gl.cxx
	gl/gl_context.cxx
	gl/gl_cursor.cxx
	gl/gl_depth_peeler.cxx
	gl/gl_image_drawable_base.cxx
	gl/gl_implicit_surface_drawable_base.cxx
	gl/gl_mesh_drawable_base.cxx
	gl/gl_texture_tools.cxx
	gl/gl_tools.cxx
	gl/gl_transparent_renderer.cxx
	gl/gl_view.cxx)

set(PH_SOURCES gl/gl_context_switch.ph)

ppp_compile("${CGV_DIR}"
		PPP_FILES 
		${PH_SOURCES})

include_directories(${GLEW_INCLUDE_PATH}
	${OPENGL_INCLUDE_DIR} 
	"${CGV_DIR}/libs"
	"${cgv_INCLUDE_DIRS}")

# FIXME: For unicode support the definitions _UNICODE and UNICODE have to be
# set. Can we do this better?
cgv_add_library(cgv_gl 
	SOURCES ${SOURCES} 
	PUBLIC_HEADERS ${HEADERS} ${PPP_FILES}
	DEFINITIONS UNICODE _UNICODE
	SHARED_DEFINITIONS CGV_RENDER_GL_EXPORTS ${GLEW_SHARED_DEFINITIONS}
	STATIC_DEFINITIONS CGV_RENDER_GL_FORCE_STATIC ${GLEW_STATIC_DEFINITIONS})

	
target_link_libraries(cgv_gl ${cgv_LIBRARIES} ${GLEW_LIBRARIES} ${OPENGL_LIBRARIES})

cgv_write_find_file(cgv_gl)

