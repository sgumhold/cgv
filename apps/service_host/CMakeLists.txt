
add_executable(service_host main.cxx)

if (CGV_LTO_ON_RELEASE)
	cgv_set_ltoflags(service_host)
endif()
set_target_properties(service_host PROPERTIES CGVPROP_TYPE "app")
set_target_properties(service_host PROPERTIES FOLDER "App")
install(TARGETS service_host EXPORT service_host DESTINATION ${CGV_BIN_DEST})

install(EXPORT service_host DESTINATION ${CGV_BIN_DEST})
