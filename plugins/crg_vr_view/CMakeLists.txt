cmake_minimum_required(VERSION 2.6)
project(crg_vr_view)

# -----------------------------------------------------------------------------
## CGV Framework ##
# The CGV framework is needed
find_package(cgv 
	COMPONENTS render gui utils type reflect data base signal
	HINTS ${CGV_DIR}
)
	
# -----------------------------------------------------------------------------
## Dependencies from CGV Framework ##
# Libraries
cgv_find_package(glsu)
cgv_find_package(cgv_gl)

cgv_find_package(vr)
cgv_find_package(cg_vr)

cgv_find_package(crg_stereo_view_lib)

# -----------------------------------------------------------------------------
# Source files
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")
# Header files
file(GLOB_RECURSE HEADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.h")


include_directories(
	"${CGV_DIR}/libs"
	${CMAKE_CURRENT_SOURCE_DIR} # source directory
	${cgv_INCLUDE_DIRS}
	${glsu_INCLUDE_DIRS}
	${cgv_gl_INCLUDE_DIRS}
	${vr_INCLUDE_DIRS}
	${cg_vr_INCLUDE_DIRS}
	${crg_stereo_view_lib_INCLUDE_DIRS}
)

#cgv_add_module(crg_vr_view ${SOURCES} ${HEADERS})
#cgv_add_export_definitions(crg_vr_view CRG_VR_VIEW_EXPORTS)
#set_target_properties(crg_vr_view PROPERTIES ENABLE_EXPORTS ON)
if(WIN32)
	cgv_add_library(crg_vr_view
		SOURCES ${SOURCES}
		PUBLIC_HEADERS ${HEADERS}
		DEFINITIONS UNICODE _UNICODE
		SHARED_DEFINITIONS CRG_VR_VIEW_EXPORTS
		STATIC_DEFINITIONS CRG_VR_VIEW_STATIC)
else()
	cgv_add_library(crg_vr_view
		SOURCES ${SOURCES}
		PUBLIC_HEADERS ${HEADERS}
		SHARED_DEFINITIONS CRG_VR_VIEW_EXPORTS
		STATIC_DEFINITIONS CRG_VR_VIEW_STATIC)
endif()
set_target_properties(crg_vr_view PROPERTIES FOLDER "${FOLDER_NAME_PLUGINS}")

#if(WIN32)
#	cgv_add_library(crg_vr_view_lib
#		SOURCES ${SOURCES}
#		PUBLIC_HEADERS ${HEADERS}
#		DEFINITIONS UNICODE _UNICODE
#		SHARED_DEFINITIONS CRG_VR_VIEW_EXPORTS
#		STATIC_DEFINITIONS CRG_VR_VIEW_STATIC)
#else()
#	cgv_add_library(crg_vr_view_lib
#		SOURCES ${SOURCES}
#		PUBLIC_HEADERS ${HEADERS}
#		SHARED_DEFINITIONS CRG_VR_VIEW_EXPORTS
#		STATIC_DEFINITIONS CRG_VR_VIEW_STATIC)
#endif()

target_link_libraries(crg_vr_view
	${cgv_LIBRARIES}
	${glsu_LIBRARIES}
	${cgv_gl_LIBRARIES}
	${vr_LIBRARIES}
	${cg_vr_LIBRARIES}
	${crg_stereo_view_lib_LIBRARIES}
)

#target_link_libraries(crg_vr_view_lib
#	${cgv_LIBRARIES}
#	${glsu_LIBRARIES}
#	${cgv_gl_LIBRARIES}
#	${vr_LIBRARIES}
#	${cg_vr_LIBRARIES}
#	${crg_stereo_view_lib_LIBRARIES}
#)

cgv_write_find_file(crg_vr_view)
#cgv_write_find_file(crg_vr_view_lib)
