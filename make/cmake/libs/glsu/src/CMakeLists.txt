cmake_minimum_required (VERSION 2.6)

add_library (glsu SHARED glsu.c glsu.def)
target_link_libraries( glsu ${OPENGL_LIBRARIES} )
install(TARGETS glsu
            RUNTIME DESTINATION bin
            LIBRARY DESTINATION lib)
install(FILES ../include/GL/glsu.h DESTINATION include/GL)
