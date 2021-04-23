
# Dependency XInput (win32) / Xinput (*nix)
if (WIN32)
    set(GAMEPAD_LIBS "XInput")
else ()
    find_package(X11 REQUIRED COMPONENTS Xinput)
    set(GAMEPAD_LIBS ${X11_LIBRARIES})
endif ()

file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")

add_library(gamepad ${SOURCES})
target_compile_definitions(gamepad PRIVATE GAMEPAD_EXPORTS)
target_link_libraries(gamepad PUBLIC ${GAMEPAD_LIBS})
target_include_directories(gamepad PUBLIC
        $<BUILD_INTERFACE:${CGV_DIR}>
        $<BUILD_INTERFACE:${CGV_DIR}/libs>
        $<INSTALL_INTERFACE:${CGV_LIBS_INCLUDE_DEST}>)

install(TARGETS gamepad EXPORT gamepad DESTINATION ${CGV_LIB_DEST})
install(EXPORT gamepad DESTINATION ${CGV_LIB_DEST})
