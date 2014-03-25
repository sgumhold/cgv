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

# Source files
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "gl/*.cxx")
# Header files
file(GLOB_RECURSE HEADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "gl/*.h")

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

