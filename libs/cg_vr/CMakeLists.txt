cmake_minimum_required(VERSION 2.6)
project(cg_vr)

# -----------------------------------------------------------------------------
## CGV Framework ##
# The CGV framework is needed
find_package(cgv COMPONENTS  utils type reflect data signal os base  math media gui HINTS ${CGV_DIR})
cgv_find_package(cgv_reflect_types)
cgv_find_package(vr REQUIRED)

# Source files
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")
# Header files
file(GLOB_RECURSE HEADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.h")

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
	"${CGV_DIR}/libs"
	${cgv_INCLUDE_DIRS}
	${cgv_reflect_types_INCLUDE_DIRS}
	${vr_INCLUDE_DIRS}
	)

# FIXME: For unicode support the definitions _UNICODE and UNICODE have to be
# set. Can we do this better?
if(WIN32)
	cgv_add_library(cg_vr 
		SOURCES ${SOURCES} 
		PUBLIC_HEADERS ${HEADERS}
		DEFINITIONS UNICODE _UNICODE
		SHARED_DEFINITIONS CG_VR_EXPORTS)
else()
	cgv_add_library(cg_vr 
		SOURCES ${SOURCES} 
		PUBLIC_HEADERS ${HEADERS}
		SHARED_DEFINITIONS CG_VR_EXPORTS)
endif()

target_link_libraries(cg_vr ${cgv_reflect_types_LIBRARIES} 	${vr_LIBRARIES} ${cgv_LIBRARIES})

cgv_write_find_file(cg_vr)

