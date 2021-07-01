
add_library(co_web web_server_impl.cxx)
target_link_libraries(co_web cgv_os webserver)
target_compile_definitions(co_web PRIVATE CGV_OS_WEB_EXPORTS)

install(TARGETS co_web EXPORT cgv_plugins DESTINATION ${CGV_BIN_DEST})
