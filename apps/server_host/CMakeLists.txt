
add_executable(server_host main.c)

if (CGV_LTO_ON_RELEASE)
	cgv_set_ltoflags(server_host)
endif()
set_target_properties(server_host PROPERTIES CGVPROP_TYPE "app")
set_target_properties(server_host PROPERTIES FOLDER "App")
install(TARGETS server_host EXPORT server_host DESTINATION ${CGV_BIN_DEST})

install(EXPORT server_host DESTINATION ${CGV_BIN_DEST})
