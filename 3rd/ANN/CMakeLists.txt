cmake_minimum_required(VERSION 3.15)

project(annf)

set(SOURCES
        ANN.cpp
        bd_fix_rad_search.cpp
        bd_pr_search.cpp
        bd_search.cpp
        bd_tree.cpp
        brute.cpp
        kd_dump.cpp
        kd_fix_rad_search.cpp
        kd_pr_search.cpp
        kd_search.cpp
        kd_split.cpp
        kd_tree.cpp
        kd_util.cpp
        perf.cpp)

install(DIRECTORY . DESTINATION ${CGV_3RD_INCLUDE_DEST}/ANN FILES_MATCHING PATTERN "*.h")

# Shared Library
add_library(ann SHARED ${SOURCES})
target_include_directories(ann PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../>
        $<INSTALL_INTERFACE:${CGV_3RD_INCLUDE_DEST}>)
target_compile_definitions(ann PRIVATE ANN_NO_RANDOM DLL_EXPORTS ANN_PERF)

add_library(annf SHARED ${SOURCES})
target_include_directories(annf PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../>
        $<INSTALL_INTERFACE:${CGV_3RD_INCLUDE_DEST}>)
target_compile_definitions(annf PRIVATE ANN_USE_FLOAT ANN_NO_RANDOM DLL_EXPORTS ANN_PERF)

install(TARGETS ann annf EXPORT cgv_3rd DESTINATION ${CGV_BIN_DEST})

# Static Library
add_library(ann_static STATIC ${SOURCES})
target_include_directories(ann_static PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../>
        $<INSTALL_INTERFACE:${CGV_3RD_INCLUDE_DEST}>)
target_compile_definitions(ann_static PRIVATE ANN_NO_RANDOM ANN_STATIC)

add_library(annf_static STATIC ${SOURCES})
target_include_directories(annf_static PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../>
        $<INSTALL_INTERFACE:${CGV_3RD_INCLUDE_DEST}>)
target_compile_definitions(annf_static PRIVATE ANN_USE_FLOAT ANN_NO_RANDOM ANN_STATIC)

# Disable warnings we can't do anything about since it would require changing the original source code
if (NOT MSVC)
        target_compile_options(ann PRIVATE -Wno-write-strings)
        target_compile_options(annf PRIVATE -Wno-write-strings)
        target_compile_options(ann_static PRIVATE -Wno-write-strings)
        target_compile_options(annf_static PRIVATE -Wno-write-strings)
        if (CMAKE_C_COMPILER_ID MATCHES "Clang" OR MAKE_CXX_COMPILER_ID MATCHES "Clang")
                target_compile_options(ann PRIVATE -Wno-deprecated-register)
                target_compile_options(annf PRIVATE -Wno-deprecated-register)
                target_compile_options(ann_static PRIVATE -Wno-deprecated-register)
                target_compile_options(annf_static PRIVATE -Wno-deprecated-register)
        endif()
endif()

install(TARGETS ann_static annf_static EXPORT cgv_3rd DESTINATION ${CGV_BIN_DEST})
