
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")
file(GLOB_RECURSE PPP_SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.ph")

ppp_compile("${CGV_DIR}"
        PPP_FILES
        PPP_INCLUDES
        PPP_INSTALL_DIR
        ${PPP_SOURCES})

add_library(cgv_type ${SOURCES} ${PPP_FILES})
target_link_libraries(cgv_type PUBLIC cgv_utils)
target_compile_definitions(cgv_type PRIVATE CGV_TYPE_EXPORTS)
target_include_directories(cgv_type PUBLIC
        $<BUILD_INTERFACE:${CGV_DIR}>
        "$<BUILD_INTERFACE:${PPP_INCLUDES}>"
        $<INSTALL_INTERFACE:include>)

install(TARGETS cgv_type EXPORT cgv_type DESTINATION ${CGV_LIB_DEST})
install(EXPORT cgv_type DESTINATION ${CGV_LIB_DEST})
install(DIRECTORY ${PPP_INSTALL_DIR} DESTINATION ${CGV_INCLUDE_DEST}/type FILES_MATCHING PATTERN "*.h")
