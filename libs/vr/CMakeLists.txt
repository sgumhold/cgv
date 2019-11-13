cmake_minimum_required(VERSION 2.6)
project(vr)

# -----------------------------------------------------------------------------
## CGV Framework ##
# The CGV framework is needed
# only for cgv_ commands
find_package(cgv COMPONENTS HINTS ${CGV_DIR})

# -----------------------------------------------------------------------------
## 3rd party dependencies ##
cgv_find_package(GLEW)
cgv_find_package(OpenGL REQUIRED)

# -----------------------------------------------------------------------------
# Source files
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")
# Header files
file(GLOB_RECURSE HEADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.h")

include_directories(
	${cgv_INCLUDE_DIRS}
	${GLEW_INCLUDE_PATH}
	${OPENGL_INCLUDE_DIR} 	
	"${CGV_DIR}/libs"
	)

# FIXME: For unicode support the definitions _UNICODE and UNICODE have to be
# set. Can we do this better?
if(WIN32)
	cgv_add_library(vr 
		SOURCES ${SOURCES} 
		PUBLIC_HEADERS ${HEADERS}
		DEFINITIONS UNICODE _UNICODE
		SHARED_DEFINITIONS VR_KIT_EXPORTS ${GLEW_SHARED_DEFINITIONS}
		STATIC_DEFINITIONS ${GLEW_STATIC_DEFINITIONS})
else()
	cgv_add_library(vr 
		SOURCES ${SOURCES} 
		PUBLIC_HEADERS ${HEADERS}
		SHARED_DEFINITIONS VR_KIT_EXPORTS ${GLEW_SHARED_DEFINITIONS}
		STATIC_DEFINITIONS ${GLEW_STATIC_DEFINITIONS})
endif()

target_link_libraries(vr ${cgv_LIBRARIES} ${GLEW_LIBRARIES} ${OPENGL_LIBRARIES})

cgv_write_find_file(vr)
