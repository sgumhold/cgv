
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")
file(GLOB_RECURSE PPP_SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.ph")

ppp_compile("${CGV_DIR}"
        PPP_FILES
        PPP_INCLUDES
        PPP_INSTALL_DIR
        ${PPP_SOURCES})

add_library(cgv_utils ${SOURCES} ${PPP_FILES})
target_compile_definitions(cgv_utils PRIVATE CGV_UTILS_EXPORTS)
target_include_directories(cgv_utils PUBLIC
        $<BUILD_INTERFACE:${CGV_DIR}>
        "$<BUILD_INTERFACE:${PPP_INCLUDES}>"
        $<INSTALL_INTERFACE:include>)

install(TARGETS cgv_utils EXPORT cgv DESTINATION ${CGV_BIN_DEST})
install(DIRECTORY ${PPP_INSTALL_DIR} DESTINATION ${CGV_INCLUDE_DEST}/utils FILES_MATCHING PATTERN "*.h")
