cmake_minimum_required(VERSION 2.8)

# Needed for building components
include("../../cmake/buildCoreComponents.cmake")


set(PUBLIC_HEADERS
	assert.h
	join.h
	quote.h)

cgv_add_core_component(defines
	HEADERS_ONLY
	PUBLIC_HEADERS ${PUBLIC_HEADERS})
