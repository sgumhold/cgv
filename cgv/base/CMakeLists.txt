
# Source files
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")
# Header files
file(GLOB_RECURSE PUBLIC_HEADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.h")

ppp_compile("${CGV_DIR}" PPP_FILES PPP_INCLUDES 
	../../cgv/type/traits/method_pointer.ph
	../../cgv/base/call_interface_impl.ph
)

add_library(cgv_base ${SOURCES})
target_include_directories(cgv_base PUBLIC
	${CGV_DIR}
	${PPP_INCLUDES}
)
target_link_libraries(cgv_base PUBLIC
	cgv_utils
	cgv_type
	cgv_data
	cgv_reflect
	cgv_signal
	${CMAKE_DL_LIBS}
)

# Definition of the component
# cgv_add_core_component(base
# 					SOURCES ${SOURCES}
# 					PUBLIC_HEADERS ${PUBLIC_HEADERS}
#	CGV_DEPENDENCIES utils type data reflect signal)

# Additional dependencies
# if (NOT WIN32)
#     target_link_libraries(cgv_base dl)
# endif()

