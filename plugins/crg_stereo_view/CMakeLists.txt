
add_library(crg_stereo_view stereo_view_interactor.cxx)
target_link_libraries(crg_stereo_view 
	cgv_render
	cgv_gui
	cgv_utils
	cgv_type
	cgv_reflect
	cgv_data
	cgv_base
	cgv_signal
	cgv_gl
	glsu
)

# cgv_add_export_definitions(crg_stereo_view CGV_RENDER_STEREO_VIEW)
#set_target_properties(crg_stereo_view PROPERTIES FOLDER "${FOLDER_NAME_PLUGINS}")
