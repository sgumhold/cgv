add_executable(index_secret
        index_secret.cxx
        ${CGV_DIR}/cgv/utils/scan.cxx
        ${CGV_DIR}/cgv/utils/advanced_scan.cxx
        ${CGV_DIR}/cgv/utils/tokenizer.cxx
        ${CGV_DIR}/cgv/utils/token.cxx
        ${CGV_DIR}/cgv/os/clipboard.cxx)
target_include_directories(index_secret PRIVATE ${CGV_DIR})
target_compile_definitions(index_secret PRIVATE CGV_FORCE_STATIC)
set_target_properties(index_secret PROPERTIES CGVPROP_TYPE "app")
set_target_properties(index_secret PROPERTIES FOLDER "App/tool")

install(TARGETS index_secret EXPORT cgv_tools DESTINATION ${CGV_BIN_DEST})
