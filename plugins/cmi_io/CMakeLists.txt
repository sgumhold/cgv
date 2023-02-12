
set(SOURCES jpg_reader.cxx png_reader.cxx png_writer.cxx tiff_reader.cxx tiff_writer.cxx)

# Shared library
add_library(cmi_io SHARED ${SOURCES})
target_link_libraries(cmi_io jpeg png tif cgv_media)
target_compile_definitions(cmi_io PRIVATE CMI_IO_EXPORTS)

install(TARGETS cmi_io EXPORT cgv_plugins DESTINATION ${CGV_BIN_DEST})

# Static library
add_library(cmi_io_static OBJECT ${SOURCES})
target_link_libraries(cmi_io_static jpeg_static png_static tif_static cgv_media_static)
target_compile_definitions(cmi_io_static PRIVATE CGV_GUI_FORCE_STATIC)

install(TARGETS cmi_io_static EXPORT cgv_plugins DESTINATION ${CGV_BIN_DEST})
