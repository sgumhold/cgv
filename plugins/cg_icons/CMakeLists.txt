cmake_minimum_required(VERSION 2.6)
project(cg_icons)

# The CGV framework is needed
find_package(cgv COMPONENTS base)
	
set(SOURCES
    main.cxx
)

include_directories(${cgv_INCLUDE_DIRS})

cgv_add_module(cg_icons ${SOURCES})
target_link_libraries(cg_icons ${cgv_LIBRARIES})

cgv_write_find_file(cg_icons)

