cmake_minimum_required(VERSION 2.6)

# Include this file for standard build settings
include("../../cmake/buildCoreComponents.cmake")


# "Normal" ANN
project(ANN)

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

set(PUBLIC_HEADERS
	ANN.h
	ANNperf.h
	ANNx.h
	bd_tree.h
	kd_fix_rad_search.h
	kd_pr_search.h
	kd_search.h
	kd_split.h
	kd_tree.h
	kd_util.h
	pr_queue.h
	pr_queue_k.h)

include_directories(./..)

cgv_add_3rd_library(ANN
	SOURCES ${SOURCES}
	PUBLIC_HEADERS ${PUBLIC_HEADERS}
	SHARED_DEFINITIONS "DLL_EXPORTS" "ANN_PERF" "ANN_NO_RANDOM"
	STATIC_DEFINITIONS "ANN_STATIC" "ANN_NO_RANDOM")

cgv_write_find_file(ANN)


# Float version of ANN
project(ANNf)

include_directories(./..)

cgv_add_3rd_library(ANNf
	SOURCES ${SOURCES}
	PUBLIC_HEADERS ${PUBLIC_HEADERS}
	SHARED_DEFINITIONS "DLL_EXPORTS" "ANN_PERF" "ANN_NO_RANDOM" "ANN_USE_FLOAT"
	STATIC_DEFINITIONS "ANN_STATIC" "ANN_NO_RANDOM" "ANN_USE_FLOAT")

cgv_write_find_file(ANNf)
