cmake_minimum_required(VERSION 2.6)
project(vr_emulator)

# -----------------------------------------------------------------------------
## CGV Framework ##
# The CGV framework is needed
find_package(cgv 
	COMPONENTS base type utils base data signal reflect render gui
	HINTS ${CGV_DIR}
)

# -----------------------------------------------------------------------------
## Dependencies from CGV Framework ##
# Libraries
cgv_find_package(glsu)
cgv_find_package(cgv_gl)
cgv_find_package(GLEW)
cgv_find_package(OpenGL REQUIRED)
cgv_find_package(vr REQUIRED)

# -----------------------------------------------------------------------------
# Source files
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")
# Header files
file(GLOB_RECURSE HEADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.h")

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
	${cgv_INCLUDE_DIRS}
	${glsu_INCLUDE_DIRS}
	${cgv_gl_INCLUDE_DIRS}
	${GLEW_INCLUDE_PATH}
	${OPENGL_INCLUDE_DIR} 
	${vr_INCLUDE_DIRS}	
)

cgv_add_module(vr_emulator ${SOURCES} ${HEADERS})

_cgv_set_definitions(vr_emulator
	COMMON CGV_FORCE_STATIC
	SHARED  ${GLEW_SHARED_DEFINITIONS} VR_EMULATOR_EXPORTS
	STATIC ${GLEW_STATIC_DEFINITIONS}
)
	
target_link_libraries(vr_emulator ${cgv_LIBRARIES} ${glsu_LIBRARIES} ${cgv_gl_LIBRARIES} ${GLEW_LIBRARIES} ${OPENGL_LIBRARIES} ${vr_LIBRARIES})

cgv_write_find_file(vr_emulator)

