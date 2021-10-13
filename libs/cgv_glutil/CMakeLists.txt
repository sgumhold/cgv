add_library(cgv_glutil SHARED
    frame_buffer_container.cxx
    path_helper.cpp
    shader_library.cxx)
target_link_libraries(cgv_glutil PUBLIC cgv_gl)
target_compile_definitions(cgv_glutil PRIVATE CGV_GLUTIL_EXPORTS)
