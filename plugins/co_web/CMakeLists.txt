
cgv_add_target(co_web
	TYPE plugin NO_EXECUTABLE
	SOURCES web_server_impl.cxx
	DEPENDENCIES cgv_os webserver
	OVERRIDE_SHARED_EXPORT_DEFINE CGV_OS_WEB_EXPORTS
)
install(TARGETS co_web EXPORT cgv_plugins DESTINATION ${CGV_BIN_DEST})
