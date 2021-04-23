
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")
file(GLOB_RECURSE PPP_SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.ph")

ppp_compile("${CGV_DIR}"
        PPP_FILES
        PPP_INCLUDES
        ${PPP_SOURCES})

add_library(cgv_signal ${SOURCES} ${PPP_FILES})
target_link_libraries(cgv_signal PUBLIC cgv_type)
target_compile_definitions(cgv_signal PRIVATE CGV_SIGNAL_EXPORTS)
target_include_directories(cgv_signal PUBLIC
        $<BUILD_INTERFACE:${CGV_DIR}>
        "$<BUILD_INTERFACE:${PPP_INCLUDES}>"
        $<INSTALL_INTERFACE:include>)

install(TARGETS cgv_signal EXPORT cgv_signal DESTINATION ${CGV_LIB_DEST})
install(EXPORT cgv_signal DESTINATION ${CGV_LIB_DEST})
list(GET PPP_INCLUDES 0 PPP_BUILD_DIR)
install(DIRECTORY ${PPP_BUILD_DIR}/cgv/signal/. DESTINATION ${CGV_INCLUDE_DEST}/signal FILES_MATCHING PATTERN "*.h")
