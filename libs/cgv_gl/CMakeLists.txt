
# Source files
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")
# Header files
file(GLOB_RECURSE HEADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.h")
# Shader files
file(GLOB_RECURSE SHADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "glsl/*.gl*")

ppp_compile("${CGV_DIR}"
	PPP_FILES 
	PPP_INCLUDES
	gl/gl_context_switch.ph
)

add_library(cgv_gl ${SOURCES})

target_include_directories(cgv_gl PUBLIC
	${GLEW_INCLUDE_PATH}
	${OPENGL_INCLUDE_DIR} 
	${CGV_DIR}/libs
	${CGV_DIR}
)
	
target_link_libraries(cgv_gl 
	cgv_utils
	cgv_type
	cgv_data
	cgv_base
	cgv_signal
	cgv_math
	cgv_media
	cgv_render
	cgv_gui
	cgv_reflect_types
	json
	glew
	${OPENGL_LIBRARIES}
)

target_compile_definitions(cgv_gl PRIVATE CGV_RENDER_GL_EXPORTS CGV_GL_EXPORTS)

# FIXME: For unicode support the definitions _UNICODE and UNICODE have to be
# set. Can we do this better?
# cgv_add_library(cgv_gl 
# 	SOURCES ${SOURCES} 
# 	PUBLIC_HEADERS ${HEADERS} ${PPP_FILES}
# 	SHADERS ${SHADERS}
# 	DEFINITIONS UNICODE _UNICODE
# 	SHARED_DEFINITIONS CGV_RENDER_GL_EXPORTS ${GLEW_SHARED_DEFINITIONS}
# 	STATIC_DEFINITIONS CGV_RENDER_GL_FORCE_STATIC ${GLEW_STATIC_DEFINITIONS})
