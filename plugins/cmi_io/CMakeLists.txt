cmake_minimum_required(VERSION 2.6)
project(cmi_io)

# The CGV framework is needed
find_package(cgv COMPONENTS base type data media HINTS $ENV{CGV_DIR})
	
cgv_find_package(ZLIB)
cgv_find_package(PNG)
cgv_find_package(JPEG)
cgv_find_package(TIFF)

set(HEADERS 
	lib_begin.h
	png_reader.h
	png_writer.h
	jpg_reader.h
	tiff_reader.h
	tiff_writer.h
)

set(SOURCES
	jpg_reader.cxx
	png_reader.cxx
	png_writer.cxx
	tiff_reader.cxx
	tiff_writer.cxx
)

include_directories(
  ${cgv_INCLUDE_DIRS}
  ${ZLIB_INCLUDE_DIRS}
  ${PNG_INCLUDE_DIRS}
  ${JPEG_INCLUDE_DIRS}
  ${TIFF_INCLUDE_DIRS}
)

cgv_add_module(cmi_io ${SOURCES} ${HEADERS})

target_link_libraries(cmi_io 
  ${cgv_LIBRARIES} 
  ${ZLIB_LIBRARIES} 
  ${PNG_LIBRARIES} 
  ${JPEG_LIBRARIES} 
  ${TIFF_LIBRARIES} 
)

cgv_add_export_definitions(cmi_io CGV_MEDIA_IMAGE_IO)

set_target_properties(cmi_io PROPERTIES FOLDER "${FOLDER_NAME_PLUGINS}")

cgv_write_find_file(cmi_io)

