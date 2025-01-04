
file(GLOB SOURCES RELATIVE "${CMAKE_CURRENT_LIST_DIR}" "src/*.cpp")
file(GLOB HEADERS RELATIVE "${CMAKE_CURRENT_LIST_DIR}" "*.h")
file(GLOB_RECURSE SHADERS RELATIVE "${CMAKE_CURRENT_LIST_DIR}" "*.gl*")

set(ADDITIONAL_INCLUDES
	"${CMAKE_CURRENT_LIST_DIR}/3rd/cpp-httplib"
	"${CMAKE_CURRENT_LIST_DIR}/3rd/earcut/include"
	"${CMAKE_CURRENT_LIST_DIR}/3rd/glm"
	"${CMAKE_CURRENT_LIST_DIR}/3rd/stb_image"
	"${CMAKE_CURRENT_LIST_DIR}/3rd/WGS84toCartesian"
	"${CGV_DIR}/3rd/cpp-httplib"
)

cgv_add_target(maptiles
	TYPE library
	SOURCES ${SOURCES}
	HEADERS ${HEADERS}

	DEPENDENCIES xml zlib cgv_gl

	ADDITIONAL_INCLUDE_PATHS ${ADDITIONAL_INCLUDES}

	OVERRIDE_SHARED_EXPORT_DEFINE MAPTILES_BUILDING
	OVERRIDE_FORCE_STATIC_DEFINE  MAPTILES_STATIC
)

# Remove include for ppp-genereated cgv/signal headers as they conflict with POSIX signal.h
if (NOT MSVC)
	get_target_property(AUTO_INCLUDES maptiles INTERFACE_INCLUDE_DIRECTORIES)
	message("----MAPTILES---- auto includes: ${AUTO_INCLUDES}")
	#string(REPLACE ${what_needs_to_go} "" new_include_dirs ${the_include_dirs})
	#set_property(DIRECTORY foo PROPERTY INCLUDE_DIRECTORIES ${new_include_dirs})
	#target_link_options(maptiles_static PRIVATE -Wno-stringop-overflow)
endif()
