cmake_minimum_required(VERSION 3.15)

project(libtorch)

message(STATUS "Looking for Torch")

# NOTES
# Install PyTorch by following these instructions: https://pytorch.org/cppdocs/installing.html
# Run CMake with CMAKE_PREFIX_PATH set to the path to the unpacked PyTorch distribution.
find_package(Torch QUIET)

if (Torch_FOUND)
    message(STATUS "Looking for Torch - found")

    add_library(libtorch
            src/libtorch.cpp)
    target_include_directories(libtorch PUBLIC
            $<BUILD_INTERFACE:${CGV_DIR}>
            "$<BUILD_INTERFACE:${TORCH_INCLUDE_DIRS}>"
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/include>
            )
    target_link_libraries(libtorch PRIVATE "${TORCH_LIBRARIES}")
    set_target_properties(libtorch PROPERTIES CGVPROP_TYPE "library")
    set_target_properties(libtorch PROPERTIES FOLDER "Lib")
else ()
    message(STATUS "Looking for Torch - not found")
endif ()
