cmake_minimum_required(VERSION 3.14)

project(GLEW)

find_package(OpenGL REQUIRED)

add_library(glew glew.c)
target_include_directories(glew PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${OPENGL_INCLUDE_DIR})
target_link_libraries(glew ${OPENGL_LIBRARIES})
target_compile_definitions(glew PRIVATE GLEW_BUILD)

# cgv_find_package(OpenGL)

# set(SOURCES
# 	glew.c)

# set(PUBLIC_HEADERS
# 	GL/glew.h
# 	GL/glxew.h
# 	GL/wglew.h)
	
# include_directories(${OPENGL_INCLUDE_DIR})

# cgv_add_3rd_library(GLEW
# 	SOURCES ${SOURCES}
# 	PUBLIC_HEADERS ${PUBLIC_HEADERS}
# 	SHARED_DEFINITIONS "GLEW_BUILD"
# 	STATIC_DEFINITIONS "GLEW_STATIC")

# target_link_libraries(GLEW
# 	${OPENGL_LIBRARIES})

# cgv_write_find_file(GLEW)
