cmake_minimum_required(VERSION 2.6)

# Needed for building components
include("../../cmake/buildCoreComponents.cmake")

# The PPP is required
find_package(ppp)

# Public headers
# TODO: To be verified
set(PUBLIC_HEADERS
	standard_types.h 
	lib_begin.h
	variant.h
	cond/has_virtual_destructor.h  
	cond/is_const.h  
	cond/is_standard_type.h
	cond/is_base_of.h              
	cond/is_enum.h
	ctrl/if_.h
	func/clean.h       
	func/drop_ref.h       
	func/make_const.h  
	func/make_return.h    
	func/promote.h
	func/drop_const.h  
	func/make_argument.h  
	func/make_ref.h    
	func/promote_const.h  
	func/transfer_const.h
	info/type_id.h  
	info/type_name.h
	info/type_access.h
	info/type_ptr.h
	traits/member_pointer.h   
	traits/min.h  
	traits/zero.h
	traits/max.h                
	traits/one.h
)

# Source files
set(SOURCES
	variant.cxx
	info/type_id.cxx  
	info/type_name.cxx
)

# Files to be preprocessed by the PPP
set(PPP_SOURCES
	invalid_type.ph  
	traits/function_pointer.ph  
	traits/method_pointer.ph
)

# Create the PPP files
ppp_compile("${CGV_DIR}"
			PPP_FILES 
			${PPP_SOURCES})

# Definition of the component
cgv_add_core_component(type
	PUBLIC_HEADERS ${PUBLIC_HEADERS} ${PPP_FILES}
	SOURCES ${SOURCES}
	CGV_DEPENDENCIES utils)

