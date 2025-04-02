include(set_plugin_execution_params.cmake)
include(cgv_target_creation.cmake)

add_subdirectory(ppp_tool)
add_subdirectory(shader_test)
add_subdirectory(res_prep)
add_subdirectory(index_secret)

install(FILES set_plugin_execution_params.cmake DESTINATION ${CGV_BIN_DEST})
install(EXPORT cgv_tools DESTINATION ${CGV_BIN_DEST})

if (WIN32)
    install(FILES run_examples.bat DESTINATION ${CGV_BIN_DEST})
else ()
    install(FILES run_examples.sh DESTINATION ${CGV_BIN_DEST})
endif ()
