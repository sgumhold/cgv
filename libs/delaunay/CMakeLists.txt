
add_library(delaunay
        corner_connectivity.cxx
        ext_corner_connectivity.cxx
        instantiation.cxx)
target_include_directories(delaunay PUBLIC ${CGV_DIR})
target_compile_definitions(delaunay PUBLIC DELAUNAY_EXPORTS)

if (UNIX)
    target_compile_options(delaunay PRIVATE -fpermissive)
endif ()
