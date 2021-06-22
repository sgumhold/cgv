
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx" "*.cpp")

add_library(rect_pack ${SOURCES})
target_compile_definitions(rect_pack PRIVATE RECT_PACK_EXPORT CGV_DIR)
target_include_directories(rect_pack PUBLIC
        $<BUILD_INTERFACE:${CGV_DIR}>
        $<BUILD_INTERFACE:${CGV_DIR}/libs>
        $<INSTALL_INTERFACE:${CGV_LIBS_INCLUDE_DEST}>)

install(TARGETS rect_pack EXPORT cgv_libs DESTINATION ${CGV_LIB_DEST})
