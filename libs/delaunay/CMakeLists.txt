cmake_minimum_required(VERSION 2.6)
project(delaunay)

# Needed for building components
include("../../cmake/buildCoreComponents.cmake")

set(HEADERS
        corner_connectivity.h
        delaunay_mesh.h
        delaunay_mesh_with_hierarchy.h
        epsilon.h
        ext_corner_connectivity.h
        instantiation.h
        lib_begin.h
        mesh_geometry.h
        mesh_geometry_reference.h
        point_2d.h
        triangle_mesh.h
)

set(SOURCES
        corner_connectivity.cxx
        ext_corner_connectivity.cxx
        instantiation.cxx
)
set(FILES
        delaunay_mesh.cxx
        delaunay_mesh_with_hierarchy.cxx
        mesh_geometry.cxx
        mesh_geometry_reference.cxx
        triangle_mesh.cxx
)
cgv_add_library(delaunay
        SOURCES ${SOURCES}
        PUBLIC_HEADERS ${HEADERS}
        FILES ${FILES}
        SHARED_DEFINITIONS DELAUNAY_EXPORTS)

if (UNIX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive")
endif()

cgv_write_find_file(delaunay)
