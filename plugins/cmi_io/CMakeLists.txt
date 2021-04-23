
add_library(cmi_io jpg_reader.cxx png_reader.cxx png_writer.cxx tiff_reader.cxx tiff_writer.cxx)
target_link_libraries(cmi_io jpeg png tif cgv_media)
target_compile_definitions(cmi_io PRIVATE CMI_IO_EXPORTS)

install(TARGETS cmi_io EXPORT cgv_plugins DESTINATION ${CGV_LIB_DEST})
