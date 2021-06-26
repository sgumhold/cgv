include(set_ide_params.cmake)

add_subdirectory(ppp_tool)
add_subdirectory(shader_test)
add_subdirectory(res_prep)

install(FILES set_ide_params.cmake DESTINATION ${CGV_LIB_DEST})
install(EXPORT cgv_tools DESTINATION ${CGV_LIB_DEST})
