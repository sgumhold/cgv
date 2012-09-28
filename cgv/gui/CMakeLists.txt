cmake_minimum_required(VERSION 2.6)

# Needed for building components
include("../../cmake/buildCoreComponents.cmake")

# Public headers
# TODO: To be verified
set(PUBLIC_HEADERS
	application.h
	base_provider_generator.h
	base_provider.h
	button.h
	control.h
	dialog.h
	event.h
	event_handler.h
	file_dialog.h
	gui_creator.h
	gui_driver.h
	gui_group.h
	key_control.h
	key_event.h
	layout.h
	layout_inline.h
	layout_spacings.h
	layout_table.h
	lib_begin.h
	menu_provider.h
	mouse_event.h
	provider.h
	resizable.h
	shortcut.h
	text_editor.h
	trigger.h
	view.h
	window.h)

# Source files
set(SOURCES
	application.cxx
	base_provider.cxx
	base_provider_generator.cxx
	button.cxx
	config_file.cxx
	control.cxx
	dialog.cxx
	event.cxx
	event_handler.cxx
	file_dialog.cxx
	fvec_gui.cxx
	gui_creator.cxx
	gui_driver.cxx
	gui_group.cxx
	key_control.cxx
	key_event.cxx
	layout.cxx
	layout_inline.cxx
	layout_spacings.cxx
	layout_table.cxx
	light_source_gui.cxx
	menu_provider.cxx
	mouse_event.cxx
	provider.cxx
	resizable.cxx
	shortcut.cxx
	text_editor.cxx
	trigger.cxx
	window.cxx)

# Definition of the component
cgv_add_core_component(gui
	SOURCES ${SOURCES}
	PUBLIC_HEADERS ${PUBLIC_HEADERS}
	CGV_DEPENDENCIES utils type reflect data base media signal)

