cmake_minimum_required(VERSION 2.6)

# Needed for building components
include("../../cmake/buildCoreComponents.cmake")

# Public headers
# TODO: To be verified
set(PUBLIC_HEADERS
	axis_aligned_box.h  
	color_model.hh  
	lib_begin.h  
	plane.h  
	sphere.h        
	color.h         
	ray.h    
	riff.h
	font/font.h  
	font/font_server.h  
	font/lib_begin.h
	illum/lib_begin.h  
	illum/light_source.hh  
	illum/obj_material.hh  
	illum/phong_material.hh
	image/lib_begin.h
	image/bmp_reader.h    
	image/bmp_writer.h    
	image/image_reader.h    
	image/image_writer.h
	mesh/dual_contouring.h  
	mesh/marching_cubes.h  
	mesh/obj_loader.h 
	mesh/obj_reader.h  
	mesh/streaming_mesh.h
	text/advanced_scan.h  
	text/callback_stream.h  
	text/convert.h  
	text/convert_string.h  
	text/scan.h  
	text/token.h  
	text/tokenizer.h
	video/lib_begin.h  
	video/video_reader.h  
	video/video_writer.h)

# Source files
set(SOURCES
	riff.cxx  
	font/font.cxx  
	font/font_server.cxx  
	illum/light_source.cxx  
	image/bmp_reader.cxx  
	image/bmp_writer.cxx  
	image/image_reader.cxx  
	image/image_writer.cxx  
	mesh/marching_cubes.cxx  
	mesh/obj_loader.cxx  
	mesh/obj_reader.cxx  
	video/video_reader.cxx  
	video/video_writer.cxx)

# Definition of the component
cgv_add_core_component(media
	SOURCES ${SOURCES}
	HEADERS ${HEADERS} 
	PUBLIC_HEADERS ${PUBLIC_HEADERS}
	CGV_DEPENDENCIES utils type data base)

# This component has multiple static and shared definitions
cgv_add_export_definitions(cgv_media
	CGV_MEDIA_FONT
	CGV_MEDIA_ILLUM
	CGV_MEDIA_IMAGE
	CGV_MEDIA_VIDEO)

