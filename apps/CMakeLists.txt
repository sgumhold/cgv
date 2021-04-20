add_subdirectory(cgv_viewer)

add_custom_target(apps)
add_dependencies(apps cgv_viewer)
