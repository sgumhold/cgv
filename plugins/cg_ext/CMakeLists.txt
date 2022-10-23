
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")

# Shared library
add_library(cg_ext SHARED ${SOURCES})
target_link_libraries(cg_ext cgv_base cgv_gui)
target_compile_definitions(cg_ext PRIVATE CGV_GUI_EXT_EXPORTS)

install(TARGETS cg_ext EXPORT cgv_plugins DESTINATION ${CGV_BIN_DEST})

# Static library
add_library(cg_ext_static STATIC ${SOURCES})
target_link_libraries(cg_ext_static PUBLIC cgv_base_static cgv_gui_static)
target_compile_definitions(cg_ext_static PUBLIC CGV_GUI_FORCE_STATIC)

install(TARGETS cg_ext_static EXPORT cgv_plugins DESTINATION ${CGV_BIN_DEST})
