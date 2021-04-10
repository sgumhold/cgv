cmake_minimum_required(VERSION 3.14)

project(delaunay)

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

add_library(delaunay ${SOURCES})
target_include_directories(delaunay PUBLIC ${CGV_DIR})

if (UNIX)
        target_compile_options(delaunay PRIVATE -fpermissive)
endif()
