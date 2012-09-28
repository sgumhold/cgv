cmake_minimum_required(VERSION 2.6)

# Needed for building components
include("../../cmake/buildCoreComponents.cmake")

# Public headers
# TODO: To be verified
set(PUBLIC_HEADERS
	context.h
	drawable.h
	frame_buffer.h
	lib_begin.h
	render_buffer.h
	shader_code.h
	shader_program.h
	texture.h
	textured_material.h
	view.h)

set(HEADERS
	../../libs/ppp/command_token.h
	../../libs/ppp/expression_processor.h
	../../libs/ppp/lib_begin.h
	../../libs/ppp/operators.h
	../../libs/ppp/ph_processor.h
	../../libs/ppp/variables.h
	../../libs/ppp/variant.h)

# Source files
set(SOURCES
	../../libs/ppp/command_token.cxx
	../../libs/ppp/expression_processor.cxx
	../../libs/ppp/operators.cxx
	../../libs/ppp/ph_processor.cxx
	../../libs/ppp/ppp_variant.cxx
	../../libs/ppp/variables.cxx
	context.cxx
	drawable.cxx
	frame_buffer.cxx
	render_buffer.cxx
	shader_code.cxx
	shader_program.cxx
	texture.cxx
	textured_material.cxx
	view.cxx)

# Definition of the component
cgv_add_core_component(render
	HEADERS ${HEADERS}
	PUBLIC_HEADERS ${PUBLIC_HEADERS}
	SOURCES ${SOURCES}
	CGV_DEPENDENCIES utils type reflect data base signal math media)

