include("pppFunctions.cmake")

# Set source files
add_executable(ppp
	ppp.cxx
	${CGV_DIR}/cgv/utils/scan.cxx
	${CGV_DIR}/cgv/utils/advanced_scan.cxx
	${CGV_DIR}/cgv/utils/tokenizer.cxx
	${CGV_DIR}/cgv/utils/token.cxx
	${CGV_DIR}/cgv/utils/file.cxx
	${CGV_DIR}/cgv/utils/dir.cxx
	${CGV_DIR}/cgv/utils/date_time.cxx
	${CGV_DIR}/cgv/ppp/command_token.cxx
	${CGV_DIR}/cgv/ppp/expression_processor.cxx
	${CGV_DIR}/cgv/ppp/operators.cxx
	${CGV_DIR}/cgv/ppp/ph_processor.cxx
	${CGV_DIR}/cgv/ppp/ppp_variant.cxx
	${CGV_DIR}/cgv/ppp/variables.cxx)
target_include_directories(ppp PRIVATE ${CGV_DIR})
