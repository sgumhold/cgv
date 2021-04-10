
# Source files
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")
# Header files
file(GLOB_RECURSE PUBLIC_HEADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.h")
# Files to be preprocessed by PPP
file(GLOB_RECURSE PPP_SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.ph")

# Create the PPP files
ppp_compile("${CGV_DIR}"
	PPP_FILES
	PPP_INCLUDES
	${PPP_SOURCES}
)

add_library(cgv_type ${SOURCES} ${PPP_FILES})
target_include_directories(cgv_type PUBLIC
	${CGV_DIR}
	${PPP_INCLUDES}
)
target_link_libraries(cgv_type PUBLIC cgv_utils)
