
add_library(delaunay
        corner_connectivity.cxx
        ext_corner_connectivity.cxx
        instantiation.cxx)
target_compile_definitions(delaunay PUBLIC DELAUNAY_EXPORTS)

if (UNIX)
    target_compile_options(delaunay PRIVATE -fpermissive)
endif ()

target_include_directories(delaunay PUBLIC
        $<BUILD_INTERFACE:${CGV_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CGV_LIBS_INCLUDE_DEST}/delaunay>)

install(TARGETS delaunay EXPORT cgv_libs DESTINATION ${CGV_LIB_DEST})
