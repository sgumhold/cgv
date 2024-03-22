
# include the 3rd party sub-libraries build tree
add_subdirectory(3rd)

# collect our own source files
# - sources
set(SOURCES
	src/stb_image.cpp
	src/OSMDataLoader.cpp
	src/OSMDataProcessor.cpp
	src/OSMRasterTileLoader.cpp
	src/OSMRasterTileProcessor.cpp
	src/RasterTileData.cpp
	src/Tile3DData.cpp
	src/TileManagerData.cpp
	src/utils.cpp
)
# -headers
set(HEADERS
	Config.h
	OSMDataLoader.h
	OSMDataProcessor.h
	OSMRasterTileLoader.h
	OSMRasterTileProcessor.h
	RasterTileData.h
	Tile3DData.h
	TileManagerData.h
	Timer.h
	utils.h
)

# create targets
add_library(maptiles SHARED ${SOURCES} ${HEADERS})
add_library(maptiles_static OBJECT ${SOURCES} ${HEADERS})
set_target_properties(maptiles PROPERTIES CGVPROP_TYPE "library")

# configure targets
# - defines
target_compile_definitions(maptiles PRIVATE MAPTILES_BUILDING=1)
target_compile_definitions(maptiles_static PRIVATE MAPTILES_BUILDING=1)
target_compile_definitions(maptiles_static PUBLIC MAPTILES_STATIC=1)
# - includes
target_include_directories(maptiles PUBLIC
	"$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
	"$<INSTALL_INTERFACE:.>"
	"$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/3rd/WGS84toCartesian>"
	"$<INSTALL_INTERFACE:3rd/WGS84toCartesian>"
)
target_include_directories(maptiles PRIVATE "${CMAKE_CURRENT_LIST_DIR}/3rd/stb_image")
target_include_directories(maptiles_static PUBLIC
	"$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
	"$<INSTALL_INTERFACE:.>"
	"$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/3rd/WGS84toCartesian>"
	"$<INSTALL_INTERFACE:3rd/WGS84toCartesian>"
)
target_include_directories(maptiles_static PRIVATE "${CMAKE_CURRENT_LIST_DIR}/3rd/stb_image")
# - dependencies
target_link_libraries(maptiles httplib earcut_hpp glm xml)
target_link_libraries(maptiles_static httplib earcut_hpp glm xml_static)

# tack on IDE fluff
# - the maptiles library itself
set_target_properties(maptiles PROPERTIES FOLDER "3rd/maptiles")
set_target_properties(maptiles_static PROPERTIES FOLDER "_obj")
# - the 3rd party sub-libraries
cgv_get_all_directory_targets(SUBTARGETS "${CMAKE_CURRENT_LIST_DIR}/3rd" RECURSIVE)
set_property(TARGET ${SUBTARGETS} PROPERTY FOLDER "3rd/maptiles/3rd")

# FIXME: CMake installs are currently still unsupported and need updating for the new CMake build system
install(TARGETS maptiles EXPORT cgv_3rd DESTINATION ${CGV_BIN_DEST})
install(TARGETS maptiles_static EXPORT cgv_libs DESTINATION ${CGV_BIN_DEST})
