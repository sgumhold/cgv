cmake_minimum_required(VERSION 2.6)
project(ppp_lib)

include("../../cmake/buildCoreComponents.cmake")

set(HEADERS
	command_token.h
	expression_processor.h
	lib_begin.h
	operators.h
	ph_processor.h
	variables.h
	variant.h)


set(SOURCES
	command_token.cxx
	expression_processor.cxx
	operators.cxx
	ph_processor.cxx
	ppp_variant.cxx
	variables.cxx
	${CGV_DIR}/cgv/utils/scan.cxx
	${CGV_DIR}/cgv/utils/advanced_scan.cxx
	${CGV_DIR}/cgv/utils/tokenizer.cxx
	${CGV_DIR}/cgv/utils/token.cxx
	${CGV_DIR}/cgv/utils/file.cxx
	${CGV_DIR}/cgv/utils/dir.cxx
	${CGV_DIR}/cgv/utils/date_time.cxx
)

cgv_add_library(ppp_lib
	SOURCES ${SOURCES}
	HEADERS ${HEADERS}
	SHARED_DEFINITIONS CGV_MEDIA_TEXT_PPP_EXPORTS CGV_UTILS_EXPORTS
	STATIC_DEFINITIONS CGV_MEDIA_TEXT_PPP_FORCE_STATIC CGV_UTILS_FORCE_STATIC)

cgv_write_find_file(ppp_lib)

