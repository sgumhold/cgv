
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")
file(GLOB_RECURSE HEADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.h")

cgv_add_target(cg_fltk
	TYPE plugin NO_EXECUTABLE
	SOURCES ${SOURCES}
	HEADERS ${HEADERS}
	DEPENDENCIES
		fltk cgv_gl cgv_gui cgv_render cgv_media cgv_utils cgv_os cgv_base
		cgv_data cgv_type cgv_signal cgv_reflect

	OVERRIDE_SHARED_EXPORT_DEFINE "CG_FLTK_EXPORTS;CGV_GUI_FLTK_EXPORTS"
	OVERRIDE_FORCE_STATIC_DEFINE  CGV_GUI_FLTK_FORCE_STATIC
)

if (NOT MSVC)
        target_compile_options(cg_fltk PRIVATE -Wno-deprecated-declarations)
        target_compile_options(cg_fltk_static PRIVATE -Wno-deprecated-declarations)
endif()


install(TARGETS cg_fltk EXPORT cgv_plugins DESTINATION ${CGV_BIN_DEST})
install(TARGETS cg_fltk_static EXPORT cgv_plugins DESTINATION ${CGV_BIN_DEST})
