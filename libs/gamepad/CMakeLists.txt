
# Dependency XInput (win32) / Xinput (*nix)
if (WIN32)
    set(GAMEPAD_LIBS "XInput")
else ()
    find_package(X11 REQUIRED COMPONENTS Xinput)
    set(GAMEPAD_LIBS ${X11_LIBRARIES})
endif ()

file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")

cgv_create_lib(gamepad SOURCES ${SOURCES})
target_link_libraries(gamepad PUBLIC ${GAMEPAD_LIBS})
target_link_libraries(gamepad_static PUBLIC ${GAMEPAD_LIBS})
