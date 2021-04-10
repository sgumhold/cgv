
add_subdirectory(ppp_tool)
add_subdirectory(shader_test)
add_subdirectory(res_prep)

add_custom_target(tools)
add_dependencies(tools ppp shader_test res_prep)
