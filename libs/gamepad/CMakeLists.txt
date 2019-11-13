cmake_minimum_required(VERSION 2.6)
project(gamepad)

# -----------------------------------------------------------------------------
## CGV Framework ##
# The CGV framework is needed
# only for cgv_ commands
find_package(cgv COMPONENTS HINTS ${CGV_DIR})

# -----------------------------------------------------------------------------
## Dependency XInput (win32) / Xinput (*nix)##
if(WIN32)
	set(GAMEPAD_LIBS "XInput")
else()
	cgv_find_package(X11 REQUIRED COMPONENTS Xinput)
	set(GAMEPAD_LIBS ${X11_LIBRARIES})
endif()

# -----------------------------------------------------------------------------
# Source files
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")
# Header files
file(GLOB_RECURSE HEADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.h")

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
	"${CGV_DIR}/libs"
	${X11_INCLUDE_DIRS}
)

# FIXME: For unicode support the definitions _UNICODE and UNICODE have to be
# set. Can we do this better?
cgv_add_library(gamepad 
	SOURCES ${SOURCES} 
	PUBLIC_HEADERS ${HEADERS}
	DEFINITIONS UNICODE _UNICODE
	SHARED_DEFINITIONS GAMEPAD_EXPORTS)

target_link_libraries(gamepad ${GAMEPAD_LIBS})

cgv_write_find_file(gamepad)
