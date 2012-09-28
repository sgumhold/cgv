cmake_minimum_required(VERSION 2.6)
project(cg_fltk)

#add_subdirectory(/home/jstaib/Entwicklung/CGV-Framework/release)

# The CGV framework is needed
find_package(cgv 
	COMPONENTS gui render media utils os base data type signal reflect
	HINTS $ENV{CGV_DIR}
	)

cgv_find_package(FLTK2)
cgv_find_package(cgv_gl)

set(HEADERS
	DockableGroup.h
	fltk_align_group.h
	fltk_base.h
	fltk_bool_control.h
	fltk_button.h
	fltk_dockable_group.h
	fltk_dragger.h
	fltk_driver.h
	fltk_driver_registry.h
	fltk_enum_control.h
	fltk_event.h
	fltk_font_server.h
	fltk_generic_window.h
	fltk_gl_view.h
	fltk_gui_group.h
	fltk_layout_group.h
	fltk_string_control.h
	fltk_string_view.h
	fltk_tab_group.h
	fltk_text_editor.h
	fltk_tree_group.h
	fltk_trigger_server.h
	fltk_value_control.h
	fltk_value_view.h
	fltk_viewer_window.h
	lib_begin.h)

set(SOURCES
	DockableGroup.cxx
	fltk_align_group.cxx
	fltk_base.cxx
	fltk_bool_control.cxx
	fltk_button.cxx
	fltk_color_control.cxx
	fltk_decorator.cxx
	fltk_dockable_group.cxx
	fltk_dragger.cxx
	fltk_enum_control.cxx
	fltk_event.cxx
	fltk_font_server.cxx
	fltk_generic_window.cxx
	fltk_gl_view.cxx
	fltk_gui_group.cxx
	fltk_layout_group.cxx
	fltk_shortcut_control.cxx
	fltk_string_control.cxx
	fltk_string_view.cxx
	fltk_tab_group.cxx
	fltk_text_editor.cxx
	fltk_tree_group.cxx
	fltk_trigger_server.cxx
	fltk_value_control.cxx
	fltk_value_view.cxx
	fltk_viewer_window.cxx
	fltk_driver.cxx
	fltk_driver_registry.cxx	
	)

include_directories( 
	"${FLTK2_INCLUDE_DIRS}" 
	"${cgv_gl_INCLUDE_DIRS}"
	"${cgv_INCLUDE_DIRS}")

cgv_add_module(cg_fltk ${SOURCES} ${HEADERS})

# FIXME: do not use internal macros
_cgv_set_definitions(cg_fltk
	SHARED CGV_GUI_FLTK_EXPORTS FL_SHARED
	STATIC CGV_GUI_FLTK_FORCE_STATIC ${cgv_gl_STATIC_DEFINITIONS})

target_link_libraries(cg_fltk ${cgv_gl_LIBRARIES} ${cgv_LIBRARIES} ${FLTK2_LIBRARIES})

cgv_write_find_file(cg_fltk)


