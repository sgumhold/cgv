cmake_minimum_required(VERSION 2.6)
project(ppp)

# Needed for building components
include("../../cmake/buildCoreComponents.cmake")

# Set source files
set(SOURCES
	ppp.cxx
	${CGV_DIR}/cgv/utils/scan.cxx
	${CGV_DIR}/cgv/utils/advanced_scan.cxx
	${CGV_DIR}/cgv/utils/tokenizer.cxx
	${CGV_DIR}/cgv/utils/token.cxx
	${CGV_DIR}/cgv/utils/file.cxx
	${CGV_DIR}/cgv/utils/dir.cxx
	${CGV_DIR}/cgv/utils/date_time.cxx
	${CGV_DIR}/libs/ppp/command_token.cxx
	${CGV_DIR}/libs/ppp/expression_processor.cxx
	${CGV_DIR}/libs/ppp/operators.cxx
	${CGV_DIR}/libs/ppp/ph_processor.cxx
	${CGV_DIR}/libs/ppp/ppp_variant.cxx
	${CGV_DIR}/libs/ppp/variables.cxx)
	
# Set the project target
cgv_add_executable(ppp ${SOURCES})

set_target_properties(ppp PROPERTIES COMPILE_DEFINITIONS CGV_FORCE_STATIC)
set_target_properties(ppp PROPERTIES FOLDER "${FOLDER_NAME_TOOL}")

cgv_write_find_file(ppp)
