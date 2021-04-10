cmake_minimum_required(VERSION 3.14)

project(res_prep)

add_executable(res_prep
	res_prep.cxx
	${CGV_DIR}/cgv/utils/scan.cxx
    ${CGV_DIR}/cgv/utils/dir.cxx
    ${CGV_DIR}/cgv/utils/file.cxx
)
target_include_directories(res_prep PRIVATE
    ${CGV_DIR}
)
