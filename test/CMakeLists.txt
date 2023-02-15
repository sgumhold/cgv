set(CGV_CONFIGURING_TESTS TRUE)

# FIXME: we need to build this test no matter what since it provides the VR emulator which some non-tests depend on
add_subdirectory(vr)

if (CGV_BUILD_TESTS)
	add_subdirectory(font_test)
	add_subdirectory(vr_draw)
	add_subdirectory(vr_lab_test)
endif()

unset(CGV_CONFIGURING_TESTS)
