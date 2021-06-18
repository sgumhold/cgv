#pragma once

#include "element_traits.h"

#include "lib_begin.h"

namespace cgv {
	namespace render {

		class CGV_API vertex_buffer;

/** the attribute_array_binding allows to define vertex attributes (i.e. the inputs to the
    vertex shader as well as an element array that stores element indices in indexed rendering.
	The vertex attributes can be set from instances of std::vector, pointers to arrays and 
    vertex_buffer objects. Each individual vertex attribute can be enabled and disabled. Each 
	vertex attribute is attached to a program location that can be queried from a shader_program
	with the get_attribute_location() function. The element array can only be set to a vertex_buffer
	object of type VBT_INDICES with the function set_element_array(). */
class CGV_API attribute_array_binding : public attribute_array_binding_base
{
	int get_attribute_location(const context& ctx, const shader_program& prog, const std::string& attr_name) const;
public:
	/**@name static interface is used to access global attribute arrays of context*/
	//@{
	/// check whether an attribute array is enabled
	static bool is_global_array_enabled(const context& ctx, int loc);
	/// enable attribute array of given location
	static bool enable_global_array(const context& ctx, int loc);
	/// disable attribute array of given location
	static bool disable_global_array(const context& ctx, int loc);
	/// point array of vertex attribute at location \c loc to vertex buffer array \c array stored in CPU memory; in case of success also enable vertex attribute array
	static bool set_global_attribute_array(const context& ctx, int loc, const vertex_buffer& vbo, type_descriptor td, size_t size, size_t offset, unsigned stride = 0);
	/// point array of vertex attribute at location \c loc to array \c array stored in CPU memory; in case of success also enable vertex attribute array
	template <typename T>
	static bool set_global_attribute_array(const context& ctx, int loc, const T& array) {
		return ctx.set_attribute_array_void(0, loc, array_descriptor_traits<T>::get_type_descriptor(array), 0, array_descriptor_traits<T>::get_address(array), array_descriptor_traits<T>::get_nr_elements(array));
	}
	/// point array of vertex attribute at location \c loc to array with \c nr_elements elements pointed to by \c array_ptr in CPU memory; in case of success also enable vertex attribute array
	template <typename T>
	static bool set_global_attribute_array(const context& ctx, int loc, const T* array_ptr, size_t nr_elements, unsigned stride = 0) {
		return ctx.set_attribute_array_void(0, loc, element_descriptor_traits<T>::get_type_descriptor(*array_ptr), 0, array_ptr, nr_elements, stride);
	}
	/// point array of vertex attribute at location \c loc to elements of given type in vertex buffer object at given offset spaced with given stride; in case of success also enable vertex attribute array
	static bool set_global_attribute_array(const context& ctx, int loc, type_descriptor element_type, const vertex_buffer& vbo, size_t offset_in_bytes, size_t nr_elements, unsigned stride_in_bytes = 0);
	/// set the global elment array to the given vertex buffer object which must me of type VBT_INDICES
	static bool set_global_element_array(const context& ctx, const vertex_buffer& vbe);
	//@}
	/// default constructor does not create attribute array binding
	attribute_array_binding();
	/// destruct attribute array binding object
	~attribute_array_binding();
	/// create the attribute array binding object
	bool create(const context& ctx);
	/// destruct attribute array binding object
	void destruct(const context& ctx);
	/// enable whole the attribute array binding object
	bool enable(context& ctx);
	/// disable whole attribute array binding object
	bool disable(context& ctx);
	/// enable array for vertex attribute at location \c loc
	bool enable_array(const context& ctx, int loc);
	/// check if array of vertex attribute at location \c loc is enabled
	bool is_array_enabled(const context& ctx, int loc) const;
	/// disable array for attribute at location \c loc
	bool disable_array(const context& ctx, int loc);
	/// set vertex attribute location to given array and enable array
	template <typename T>
	bool set_attribute_array(const context& ctx, int loc, const T& array) {
		return ctx.set_attribute_array_void(this, loc, array_descriptor_traits<T>::get_type_descriptor(array), 0, array_descriptor_traits<T>::get_address(array), array_descriptor_traits<T>::get_nr_elements(array));
	}
	/// set a vertex attribute to a single value or an array of values through the cgv::math::vec or std::vector classes.
	template <typename T>
	bool set_attribute_array(const context& ctx, int loc, const T* value_ptr, unsigned nr_elements, unsigned stride = 0) {
		return ctx.set_attribute_array_void(this, loc, get_element_type(*value_ptr), 0, value_ptr, nr_elements, stride);
	}
	/// point array of vertex attribute at location \c loc to elements of given type in vertex buffer object at given offset spaced with given stride; in case of success also enable vertex attribute array
	bool set_attribute_array(const context& ctx, int loc, type_descriptor element_type, const vertex_buffer& vbo, size_t offset_in_bytes, size_t nr_elements, unsigned stride_in_bytes = 0);
	/// convenience function that determines attribute location in program by name and then uses set_attribute_array to set vertex attribute location to given array and enable array
	template <typename T>
	bool bind_attribute_array(const context& ctx, const shader_program& prog, const std::string& attribute_name, const T& array) {
		int loc = get_attribute_location(ctx, prog, attribute_name);
		if (loc == -1)
			return false;
		return ctx.set_attribute_array_void(this, loc, array_descriptor_traits<T>::get_type_descriptor(array), 0, array_descriptor_traits<T>::get_address(array), array_descriptor_traits<T>::get_nr_elements(array));
	}
	/// convenience function that determines attribute location in program by name and then uses set_attribute_array to set a vertex attribute to a single value or an array of values through the cgv::math::vec or std::vector classes.
	template <typename T>
	bool bind_attribute_array(const context& ctx, const shader_program& prog, const std::string& attribute_name, const T* value_ptr, unsigned nr_elements, unsigned stride = 0) {
		int loc = get_attribute_location(ctx, prog, attribute_name);
		if (loc == -1)
			return false;
		return ctx.set_attribute_array_void(this, loc, get_element_type(*value_ptr), 0, value_ptr, nr_elements, stride);
	}
	/// convenience function that determines attribute location in program by name and then uses set_attribute_array to point array of vertex attribute at location \c loc to elements of given type in vertex buffer object at given offset spaced with given stride
	bool bind_attribute_array(const context& ctx, 
		const shader_program& prog, const std::string& attribute_name, 
		type_descriptor element_type, const vertex_buffer& vbo, 
		size_t offset_in_bytes, size_t nr_elements, unsigned stride_in_bytes = 0);
	/// set the elment array to the given vertex buffer object which must me of type VBT_INDICES
	bool set_element_array(const context& ctx, const vertex_buffer& vbe);
};

	}
}

#include <cgv/config/lib_end.h>
