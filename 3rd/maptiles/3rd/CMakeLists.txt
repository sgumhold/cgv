
# manage inclusion of 3rd party sublibs
# - cpp-httplib
set(HTTPLIB_REQUIRE_OPENSSL OFF CACHE BOOL "" FORCE)
mark_as_advanced(FORCE HTTPLIB_REQUIRE_OPENSSL)
set(HTTPLIB_REQUIRE_ZLIB OFF CACHE BOOL "" FORCE) # we will still use zlib; the internally provided one will be force-fed in
mark_as_advanced(FORCE HTTPLIB_REQUIRE_ZLIB)
set(HTTPLIB_USE_OPENSSL_IF_AVAILABLE OFF CACHE BOOL "" FORCE)
mark_as_advanced(FORCE HTTPLIB_USE_OPENSSL_IF_AVAILABLE)
set(HTTPLIB_USE_ZLIB_IF_AVAILABLE OFF CACHE BOOL "" FORCE) # we will still use zlib; the internally provided one will be force-fed in
mark_as_advanced(FORCE HTTPLIB_USE_ZLIB_IF_AVAILABLE)
set(HTTPLIB_COMPILE OFF CACHE BOOL "" FORCE)
mark_as_advanced(FORCE HTTPLIB_COMPILE)
set(HTTPLIB_TEST OFF CACHE BOOL "" FORCE)
mark_as_advanced(FORCE HTTPLIB_TEST)
set(HTTPLIB_REQUIRE_BROTLI OFF CACHE BOOL "" FORCE)
mark_as_advanced(FORCE HTTPLIB_REQUIRE_BROTLI)
set(HTTPLIB_USE_BROTLI_IF_AVAILABLE OFF CACHE BOOL "" FORCE)
mark_as_advanced(FORCE HTTPLIB_USE_BROTLI_IF_AVAILABLE)
add_subdirectory(cpp-httplib)
# - earcut
set(EARCUT_BUILD_TESTS OFF CACHE BOOL "" FORCE)
mark_as_advanced(FORCE EARCUT_BUILD_TESTS)
set(EARCUT_BUILD_BENCH OFF CACHE BOOL "" FORCE)
mark_as_advanced(FORCE EARCUT_BUILD_BENCH)
set(EARCUT_BUILD_VIZ OFF CACHE BOOL "" FORCE)
mark_as_advanced(FORCE EARCUT_BUILD_VIZ)
add_subdirectory(earcut)
# - glm
add_subdirectory(glm)
install(TARGETS glm EXPORT cgv_3rd DESTINATION ${CGV_BIN_DEST})
# - stb_image
#<nothing to be done, since this doesn't provide a CMakeLists.txt, will be directly inserted into the maptiles build instead>
# - tinyxml2
#set(tinyxml2_BUILD_TESTING OFF CACHE BOOL "" FORCE)
#mark_as_advanced(FORCE tinyxml2_BUILD_TESTING)
#add_subdirectory(tinyxml2)
# - WGS84toCartesian
#<nothing to be done, since this doesn't provide a CMakeLists.txt, will be directly inserted into the maptiles build instead>
