
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")
file(GLOB_RECURSE PPP_SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.ph")

install(DIRECTORY ${PPP_INSTALL_DIR} DESTINATION ${CGV_INCLUDE_DEST}/base FILES_MATCHING PATTERN "*.h")

cgv_create_lib(cgv_base CORE_LIB SOURCES ${SOURCES} PPP_SOURCES ${PPP_SOURCES}
        DEPENDENCIES cgv_utils cgv_type cgv_data cgv_reflect cgv_signal)
target_link_libraries(cgv_base PUBLIC ${CMAKE_DL_LIBS})
target_link_libraries(cgv_base_static PUBLIC ${CMAKE_DL_LIBS})
