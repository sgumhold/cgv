
# Source files
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")
# Header files
file(GLOB_RECURSE PUBLIC_HEADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.h")

add_library(cgv_os ${SOURCES})
target_include_directories(cgv_os PUBLIC
	${CGV_DIR}
)
target_link_libraries(cgv_os PUBLIC cgv_utils)

# if (UNIX)
# 	set(CMAKE_CXX_FLAGS "-std=c++11")
# endif()

# FIXME what is this for?
# add defines with the cgv-directory and the renderer_test directory
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DCGV_DIR=\"${CGV_SOURCE_DIR}\" -DINPUT_DIR=\"${CMAKE_CURRENT_LIST_DIR}\"")
	
# Additional dependencies
# if (WIN32)
#   In windows we also need ws2_32
#	target_link_libraries(cgv_os ${pthread_LIBRARIES})
# 	target_link_libraries(cgv_os ws2_32)
# else()
# 	target_link_libraries(cgv_os ${CMAKE_THREAD_LIBS_INIT})
# endif()
