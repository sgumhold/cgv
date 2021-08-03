
set(SOURCES
        corner_connectivity.cxx
        ext_corner_connectivity.cxx
        instantiation.cxx)

cgv_create_lib(delaunay SOURCES ${SOURCES})
