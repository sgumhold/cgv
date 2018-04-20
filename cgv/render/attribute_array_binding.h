#pragma once

#include "element_traits.h"

#include "lib_begin.h"

namespace cgv {
	namespace render {

		class vertex_buffer;

/** a shader program combines several shader code fragments
    to a complete definition of the shading pipeline. */
class CGV_API attribute_array_binding : public attribute_array_binding_base
{
public:
	/**@name static interface is used to access global attribute arrays of context*/
	//@{
	/// check whether an attribute array is enabled
	static bool is_global_array_enabled(context& ctx, int loc);
	/// enable attribute array of given location
	static bool enable_global_array(context& ctx, int loc);
	/// disable attribute array of given location
	static bool disable_global_array(context& ctx, int loc);
	/// point array of vertex attribute at location \c loc to vertex buffer array \c array stored in CPU memory; in case of success also enable vertex attribute array
	static bool set_global_attribute_array(context& ctx, int loc, const vertex_buffer& vbo, type_descriptor td, size_t size, size_t offset, size_t stride = 0);
	/// point array of vertex attribute at location \c loc to array \c array stored in CPU memory; in case of success also enable vertex attribute array
	template <typename T>
	static bool set_global_attribute_array(context& ctx, int loc, const T& array) {
		return ctx.set_attribute_array_void(0, loc, array_descriptor_traits<T>::get_type_descriptor(array), 0, array_descriptor_traits<T>::get_address(array), array_descriptor_traits<T>::get_nr_elements(array));
	}
	/// point array of vertex attribute at location \c loc to array with \c nr_elements elements pointed to by \c array_ptr in CPU memory; in case of success also enable vertex attribute array
	template <typename T>
	static bool set_global_attribute_array(context& ctx, int loc, const T* array_ptr, unsigned nr_elements, unsigned stride = 0) {
		return ctx.set_attribute_array_void(0, loc, element_descriptor_traits<T>::get_type_descriptor(*array_ptr), 0, array_ptr, nr_elements, stride);
	}
	/// point array of vertex attribute at location \c loc to elements of given type in vertex buffer object at given offset spaced with given stride; in case of success also enable vertex attribute array
	static bool set_global_attribute_array(context& ctx, int loc, type_descriptor element_type, const vertex_buffer& vbo, size_t offset_in_bytes, size_t nr_elements, size_t stride_in_bytes = 0);
	//@}
	/// default constructor does not create attribute array binding
	attribute_array_binding();
	/// destruct attribute array binding object
	~attribute_array_binding();
	/// create the attribute array binding object
	bool create(context& ctx);
	/// destruct attribute array binding object
	void destruct(context& ctx);
	/// enable whole the attribute array binding object
	bool enable(context& ctx);
	/// disable whole attribute array binding object
	bool disable(context& ctx);
	/// enable array for vertex attribute at location \c loc
	bool enable_array(context& ctx, int loc);
	/// check if array of vertex attribute at location \c loc is enabled
	bool is_array_enabled(context& ctx, int loc) const;
	/// disable array for attribute at location \c loc
	bool disable_array(context& ctx, int loc);
	/// set vertex attribute location to given array and enable array
	template <typename T>
	bool set_attribute_array(context& ctx, int loc, const T& array) {
		return ctx.set_attribute_array_void(this, loc, array_descriptor_traits<T>::get_type_descriptor(array), 0, array_descriptor_traits<T>::get_address(array), array_descriptor_traits<T>::get_nr_elements(array));
	}
	/// set a vertex attribute to a single value or an array of values through the cgv::math::vec or std::vector classes.
	template <typename T>
	bool set_attribute_array(context& ctx, int loc, const T* value_ptr, unsigned nr_elements, unsigned stride = 0) {
		return ctx.set_attribute_array_void(this, loc, element_descriptor_traits<T>::get_type_descriptor(*value_ptr), 0, value_ptr, nr_elements, stride);
	}
	/// point array of vertex attribute at location \c loc to elements of given type in vertex buffer object at given offset spaced with given stride; in case of success also enable vertex attribute array
	bool set_attribute_array(context& ctx, int loc, type_descriptor element_type, const vertex_buffer& vbo, size_t offset_in_bytes, size_t nr_elements, size_t stride_in_bytes = 0);
};

	}
}

#include <cgv/config/lib_end.h>
