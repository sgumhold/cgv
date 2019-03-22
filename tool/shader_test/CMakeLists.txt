cmake_minimum_required(VERSION 2.6)
project(shader_test)

# Needed for building components
include("../../cmake/buildCoreComponents.cmake")

# Find the PPP
find_package(ppp)

# Find 3rd party dependencies
find_package(GLEW REQUIRED)
find_package(OpenGL REQUIRED)

# In Windows FLTK2 is not needed
if (NOT WIN32)
	find_package(FLTK2 REQUIRED)
else()
	set(FLTK2_LIBRARIES "")
endif()
	
# Add library path to the list of includes
include_directories(${CGV_DIR}/libs
					${FLTK2_INCLUDE_DIRS}
					${GLEW_INCLUDE_DIRS}
					${CGV_DIR}/3rd/glew
					${CGV_DIR})

file(GLOB SOURCES_UTILS "${CGV_BASE}/cgv/utils/*.cxx")
file(GLOB_RECURSE SOURCES_TYPE "${CGV_BASE}/cgv/type/*.cxx")
file(GLOB SOURCES_REFLECT "${CGV_BASE}/cgv/reflect/*.cxx")
file(GLOB SOURCES_DATA "${CGV_BASE}/cgv/data/*.cxx")
file(GLOB SOURCES_BASE "${CGV_BASE}/cgv/base/*.cxx")
file(GLOB SOURCES_SIGNAL "${CGV_BASE}/cgv/signal/*.cxx")
file(GLOB SOURCES_PPP "${CGV_BASE}/libs/ppp/*.cxx")



set(SOURCES
	shader_test.cxx
	"${CGV_BASE}/cgv/gui/file_dialog.cxx"
    "${CGV_BASE}/cgv/gui/gui_driver.cxx"
    "${CGV_BASE}/cgv/gui/control.cxx"
    "${CGV_BASE}/cgv/media/image/image_writer.cxx"
    "${CGV_BASE}/cgv/media/font/font.cxx"
    "${CGV_BASE}/cgv/media/illum/phong_material.cxx"
    "${CGV_BASE}/cgv/media/illum/surface_material.cxx"
    "${CGV_BASE}/cgv/media/illum/light_source.cxx"
    "${CGV_BASE}/cgv/media/font/font_server.cxx"
    "${CGV_BASE}/cgv/os/clipboard.cxx"
    "${CGV_BASE}/cgv/render/context.cxx"
    "${CGV_BASE}/cgv/render/drawable.cxx"
    "${CGV_BASE}/libs/cgv_gl/gl/gl.cxx"
    "${CGV_BASE}/libs/cgv_gl/gl/gl_texture_tools.cxx"
    "${CGV_BASE}/libs/cgv_gl/gl/gl_context.cxx"
    "${CGV_BASE}/libs/cgv_gl/gl/gl_context_win32.cxx"
    "${CGV_BASE}/cgv/render/shader_code.cxx"
    "${CGV_BASE}/cgv/render/frame_buffer.cxx"
    "${CGV_BASE}/cgv/render/attribute_array_binding.cxx"
    "${CGV_BASE}/cgv/render/render_buffer.cxx"
    "${CGV_BASE}/cgv/render/shader_program.cxx"
	${SOURCES_UTILS}
	${SOURCES_TYPE}
	${SOURCES_REFLECT}
	${SOURCES_DATA}
	${SOURCES_BASE}
	${SOURCES_SIGNAL}
	${SOURCES_PPP})

if (WIN32)
	set(SOURCES ${SOURCES}
		"${CGV_BASE}/libs/cgv_gl/gl/gl_context_win32.cxx")
endif()
	

set(PH_SOURCES
	../../libs/cgv_gl/gl/gl_context_switch.ph
	../../cgv/type/traits/method_pointer.ph
	../../cgv/type/invalid_type.ph
	../../cgv/signal/signal.ph
	../../cgv/signal/abst_signal.ph
	../../cgv/signal/bool_signal.ph
	../../cgv/utils/ostream_printf.ph)

ppp_compile("${CGV_DIR}" 
			PPP_FILES 
			${PH_SOURCES})

cgv_add_executable(shader_test ${SOURCES} ${PPP_FILES})

# FIXME: do not use internal macros
_cgv_set_definitions(shader_test
	COMMON CGV_FORCE_STATIC
	STATIC ${GLEW_STATIC_DEFINITIONS})


target_link_libraries(shader_test 
	${GLEW_LIBRARY} 
	${GLEW_LIBRARIES}
	${OPENGL_LIBRARIES}
	${FLTK2_LIBRARIES}
)

if (NOT WIN32)
	target_link_libraries(shader_test dl)
endif()

set_target_properties(shader_test PROPERTIES FOLDER "${FOLDER_NAME_TOOL}")

cgv_write_find_file(shader_test)

