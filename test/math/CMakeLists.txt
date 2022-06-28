include(GoogleTest)

add_executable(test_intersection test_intersection.cpp)
target_link_libraries(test_intersection gtest_main cgv_math)

if (CGV_ENABLE_TESTS)
    gtest_discover_tests(test_intersection)
endif ()
