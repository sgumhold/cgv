find_package(OpenGL)
find_package(JPEG)
find_package(PNG)

if (unix)
	find_package(X11)
endif()

if (CGV_INSTALL_DIR)
	include("${CGV_INSTALL_DIR}/${INSTALL_CMAKE_PATH}/FindPackageMacros.cmake")
else()
	include("${CMAKE_CURRENT_LIST_DIR}/../../cmake/FindPackageMacros.cmake")
endif()

cgv_find_library(FLTK)

# Expand the libraries to contain the dependencies
set(FLTK_LIBRARIES 
	"${FLTK_LIBRARIES}" 
	"${JPEG_LIBRARIES}" 
	"${PNG_LIBRARIES}"
	"${X11_Xi_LIB}")

