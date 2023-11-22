include(res_prep.cmake)

add_executable(res_prep
        res_prep.cxx
        ${CGV_DIR}/cgv/utils/scan.cxx
        ${CGV_DIR}/cgv/utils/dir.cxx
        ${CGV_DIR}/cgv/utils/file.cxx)
target_include_directories(res_prep PRIVATE ${CGV_DIR})
target_compile_definitions(res_prep PRIVATE CGV_FORCE_STATIC)
set_target_properties(res_prep PROPERTIES CGVPROP_TYPE "app")
set_target_properties(res_prep PROPERTIES FOLDER "App/tool")

install(TARGETS res_prep EXPORT cgv_tools DESTINATION ${CGV_BIN_DEST})
