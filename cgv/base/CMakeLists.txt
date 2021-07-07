
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")
file(GLOB_RECURSE PPP_SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.ph")

ppp_compile("${CGV_DIR}"
        PPP_FILES
        PPP_INCLUDES
        PPP_INSTALL_DIR
        ${PPP_SOURCES})

install(DIRECTORY ${PPP_INSTALL_DIR} DESTINATION ${CGV_INCLUDE_DEST}/base FILES_MATCHING PATTERN "*.h")

cgv_create_core_lib(cgv_base SOURCES ${SOURCES} PPP_FILES ${PPP_FILES} PPP_INCLUDES ${PPP_INCLUDES}
        DEPENDENCIES cgv_utils cgv_type cgv_data cgv_reflect cgv_signal)
target_link_libraries(cgv_base PUBLIC ${CMAKE_DL_LIBS})
