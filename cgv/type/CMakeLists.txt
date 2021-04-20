
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")
file(GLOB_RECURSE PPP_SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.ph")

ppp_compile("${CGV_DIR}"
        PPP_FILES
        PPP_INCLUDES
        ${PPP_SOURCES})

add_library(cgv_type ${SOURCES} ${PPP_FILES})
target_compile_definitions(cgv_type PRIVATE CGV_TYPE_EXPORTS)
target_include_directories(cgv_type PUBLIC ${CGV_DIR} ${PPP_INCLUDES})
target_link_libraries(cgv_type PUBLIC cgv_utils)
