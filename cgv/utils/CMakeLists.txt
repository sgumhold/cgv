cmake_minimum_required(VERSION 2.6)

# Needed for building components
include("../../cmake/buildCoreComponents.cmake")

# The PPP is required
find_package(ppp)


# Public headers
# TODO: To be verified
set(PUBLIC_HEADERS
	advanced_scan.h
	big_binary_file.h
	bit_operations.h
	convert.h
	convert_string.h
	date_time.h
	dir.h
	file.h
	lib_begin.h
	progression.h
	scan_enum.h
	scan.h
	statistics.h
	stopwatch.h
	token.h
	tokenizer.h)

# Source files
set(SOURCES
	advanced_scan.cxx
	big_binary_file.cxx
	bit_operations.cxx
	convert.cxx
	convert_string.cxx
	date_time.cxx
	dir.cxx
	file.cxx
	ostream_printf.cxx
	progression.cxx
	scan.cxx
	scan_enum.cxx
	statistics.cxx
	stopwatch.cxx
	token.cxx
	tokenizer.cxx)

# Files to be preprocessed by the PPP
set(PPP_SOURCES
	ostream_printf.ph)

# Create the PPP files
ppp_compile("${CGV_DIR}"
			PPP_FILES 
			${PPP_SOURCES})

# Definition of the component
cgv_add_core_component(utils
	PUBLIC_HEADERS ${PUBLIC_HEADERS} ${PPP_FILES}
	SOURCES ${SOURCES})

