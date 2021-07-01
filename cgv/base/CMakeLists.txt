
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")
file(GLOB_RECURSE PPP_SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.ph")

ppp_compile("${CGV_DIR}"
        PPP_FILES
        PPP_INCLUDES
        PPP_INSTALL_DIR
        ${PPP_SOURCES})

add_library(cgv_base ${SOURCES} ${PPP_FILES})
target_compile_definitions(cgv_base PRIVATE CGV_BASE_EXPORTS)
target_link_libraries(cgv_base PUBLIC
        cgv_utils
        cgv_type
        cgv_data
        cgv_reflect
        cgv_signal
        ${CMAKE_DL_LIBS})
target_include_directories(cgv_base PUBLIC
        $<BUILD_INTERFACE:${CGV_DIR}>
        "$<BUILD_INTERFACE:${PPP_INCLUDES}>"
        $<INSTALL_INTERFACE:include>)

install(TARGETS cgv_base EXPORT cgv DESTINATION ${CGV_BIN_DEST})
install(DIRECTORY ${PPP_INSTALL_DIR} DESTINATION ${CGV_INCLUDE_DEST}/base FILES_MATCHING PATTERN "*.h")
