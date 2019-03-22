cmake_minimum_required(VERSION 2.6)
project(crg_light)

# The CGV framework is needed
find_package(cgv COMPONENTS utils type reflect data base signal media render gui)

cgv_find_package(cgv_gl REQUIRED)

set(HEADERS 
	lib_begin.h
	light_interactor.h
)

set(SOURCES
	light_interactor.cxx
)

include_directories(
  ${cgv_INCLUDE_DIRS}
  ${cgv_gl_INCLUDE_DIRS}
)

cgv_add_module(crg_light ${SOURCES} ${HEADERS})
target_link_libraries(crg_light 
  ${cgv_LIBRARIES}
  ${cgv_gl_LIBRARIES}
)

cgv_add_export_definitions(crg_light CGV_RENDER_LIGHT)

cgv_write_find_file(crg_light)

