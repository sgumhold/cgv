cmake_minimum_required(VERSION 2.6)
project(cgv_viewer)

find_package(cgv COMPONENTS signal base gui)
include_directories(${cgv_INCLUDE_DIRS})
set(SOURCES main.cxx)

cgv_add_executable(cgv_viewer ${SOURCES})
set_target_properties(cgv_viewer PROPERTIES FOLDER "${FOLDER_NAME_APPS}")
target_link_libraries(cgv_viewer ${cgv_LIBRARIES})
cgv_write_find_file(cgv_viewer)

# We need to install the main file for the binary distribution
install(FILES main.cxx 
		DESTINATION "${INSTALL_BASE}/${INSTALL_HEADER_PATH}/apps/cgv_viewer")
