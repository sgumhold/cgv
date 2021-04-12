
add_library(crg_grid grid.cxx main.cxx)
target_include_directories(crg_grid PUBLIC ../../libs)
target_link_libraries(crg_grid 
	cgv_render
	cgv_gui
	cgv_base
	cgv_type
	cgv_reflect
	cgv_utils
	cgv_signal
	cgv_data
	cgv_gl
)
target_compile_definitions(crg_grid PRIVATE _USRDLL)
