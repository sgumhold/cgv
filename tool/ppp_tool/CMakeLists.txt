include(ppp.cmake)

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
target_compile_definitions(ppp PRIVATE CGV_FORCE_STATIC)
set_target_properties(ppp PROPERTIES CGVPROP_TYPE "app")
set_target_properties(ppp PROPERTIES FOLDER "App/tool")

install(TARGETS ppp EXPORT cgv_tools DESTINATION ${CGV_BIN_DEST})
