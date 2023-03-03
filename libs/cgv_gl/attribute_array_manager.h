#pragma once

#include <cgv/render/context.h>
#include <cgv/render/vertex_buffer.h>
#include <cgv/render/attribute_array_binding.h>
#include <cgv_gl/gl/gl_context.h>

#include "gl/lib_begin.h"

namespace cgv { // @<
namespace render { // @<

/// attribute array manager used to upload arrays to gpu
class CGV_API attribute_array_manager {
protected:
	/// store default buffer usage
	VertexBufferUsage default_usage;
	/// attribue array binding used to store array pointers
	attribute_array_binding aab;
	/// store vertex buffers generated per attribute location
	std::map<int, vertex_buffer*> vbos;
	/// give renderer access to protected members
	friend class renderer;
public:
	/// 
	template <typename T>
	bool set_indices(const context& ctx, const T& array) {
		bool res;
		vertex_buffer*& vbo_ptr = vbos[-1];
		if(vbo_ptr) {
			if(vbo_ptr->get_size_in_bytes() == array_descriptor_traits <T>::get_size(array))
				res = vbo_ptr->replace(ctx, 0, array_descriptor_traits <T>::get_address(array), array_descriptor_traits < T>::get_nr_elements(array));
			else {
				vbo_ptr->destruct(ctx);
				res = vbo_ptr->create(ctx, array);
			}
		} else {
			vbo_ptr = new vertex_buffer(VBT_INDICES, default_usage);
			res = vbo_ptr->create(ctx, array);
		}
		if(res)
			res = ctx.set_element_array(&aab, vbo_ptr);
		return res;
	}
	/// 
	template <typename T>
	bool set_indices(const context& ctx, const T* array, size_t count) {
		bool res;
		vertex_buffer*& vbo_ptr = vbos[-1];
		if(vbo_ptr) {
			if(vbo_ptr->get_size_in_bytes() == count * get_type_size(cgv::type::info::type_id<T>::get_id()))
				res = vbo_ptr->replace(ctx, 0, array, count);
			else {
				vbo_ptr->destruct(ctx);
				res = vbo_ptr->create(ctx, array, count);
			}
		} else {
			vbo_ptr = new vertex_buffer(VBT_INDICES, default_usage);
			res = vbo_ptr->create(ctx, array, count);
		}
		if(res)
			res = ctx.set_element_array(&aab, vbo_ptr);
		return res;
	}
	/// whether aam contains an index buffer
	bool has_index_buffer() const;
	///
	void remove_indices(const context& ctx);
	///
	template <typename T>
	bool set_attribute_array(const context& ctx, int loc, const T& array) {
		bool res;
		vertex_buffer*& vbo_ptr = vbos[loc];
		if(vbo_ptr) {
			if(vbo_ptr->get_size_in_bytes() == array_descriptor_traits <T>::get_size(array))
				res = vbo_ptr->replace(ctx, 0, array_descriptor_traits <T>::get_address(array), array_descriptor_traits < T>::get_nr_elements(array));
			else {
				vbo_ptr->destruct(ctx);
				res = vbo_ptr->create(ctx, array);
			}
		} else {
			vbo_ptr = new vertex_buffer(VBT_VERTICES, default_usage);
			res = vbo_ptr->create(ctx, array);
		}
		if(res)
			res = ctx.set_attribute_array_void(&aab, loc, array_descriptor_traits <T>::get_type_descriptor(array), vbo_ptr, 0, array_descriptor_traits < T>::get_nr_elements(array));
		return res;
	}
	///
	template <typename T>
	bool set_attribute_array(const context& ctx, int loc, const T* array_ptr, size_t nr_elements, unsigned stride) {
		bool res;
		vertex_buffer*& vbo_ptr = vbos[loc];
		if(vbo_ptr) {
			if(vbo_ptr->get_size_in_bytes() == nr_elements * sizeof(T))
				res = vbo_ptr->replace(ctx, 0, array_ptr, nr_elements);
			else {
				vbo_ptr->destruct(ctx);
				res = vbo_ptr->create(ctx, array_ptr, nr_elements);
			}
		} else {
			vbo_ptr = new vertex_buffer(VBT_VERTICES, default_usage);
			res = vbo_ptr->create(ctx, array_ptr, nr_elements);
		}
		if(res)
			res = ctx.set_attribute_array_void(&aab, loc, type_descriptor(element_descriptor_traits<T>::get_type_descriptor(*array_ptr), true), vbo_ptr, 0, nr_elements);
		return res;
	}
	///
	bool set_attribute_array(const context& ctx, int loc, type_descriptor element_type, const vertex_buffer& vbo, size_t offset_in_bytes, size_t nr_elements, unsigned stride_in_bytes);

	template <typename C, typename T>
	bool set_composed_attribute_array(const context& ctx, int loc, const C* array_ptr, size_t nr_elements, const T& elem) {
		bool res;
		vertex_buffer*& vbo_ptr = vbos[loc];
		if(vbo_ptr) {
			if(vbo_ptr->get_size_in_bytes() == nr_elements * sizeof(C))
				res = vbo_ptr->replace(ctx, 0, array_ptr, nr_elements);
			else {
				vbo_ptr->destruct(ctx);
				res = vbo_ptr->create(ctx, array_ptr, nr_elements);
			}
		} else {
			vbo_ptr = new vertex_buffer(VBT_VERTICES, default_usage);
			res = vbo_ptr->create(ctx, array_ptr, nr_elements);
		}
		if(res)
			res = ctx.set_attribute_array_void(&aab, loc,
				type_descriptor(element_descriptor_traits<T>::get_type_descriptor(elem), true),
				vbo_ptr,
				reinterpret_cast<const void*>(reinterpret_cast<const cgv::type::uint8_type*>(&elem) - reinterpret_cast<const cgv::type::uint8_type*>(array_ptr)),
				nr_elements, sizeof(C));
		return res;
	}
	template <typename C, typename T>
	bool ref_composed_attribute_array(const context& ctx, int loc, int loc_ref, const C* array_ptr, size_t nr_elements, const T& elem) {
		vertex_buffer*& vbo_ptr = vbos[loc_ref];
		if(!vbo_ptr)
			return false;
		return ctx.set_attribute_array_void(&aab, loc,
			type_descriptor(element_descriptor_traits<T>::get_type_descriptor(elem), true),
			vbo_ptr,
			reinterpret_cast<const void*>(reinterpret_cast<const cgv::type::uint8_type*>(&elem) - reinterpret_cast<const cgv::type::uint8_type*>(array_ptr)),
			nr_elements, sizeof(C));
	}
public:
	/// default initialization
	attribute_array_manager(VertexBufferUsage _default_usage = VBU_STREAM_DRAW);
	/// destructor calls destruct
	~attribute_array_manager();
	/// check whether the given attribute is available
	bool has_attribute(const context& ctx, int loc) const;
	/// returns the pointer to the vertex buffer as managed by this attribute array manager if specified and nullptr otherwise
	const vertex_buffer* get_buffer_ptr(int loc) const {
		auto it = vbos.find(loc);
		if(it != vbos.end())
			return it->second;
		return nullptr;
	}
	///
	bool is_created();
	///
	bool init(context& ctx);
	///
	bool enable(context& ctx);
	///
	bool disable(context& ctx);
	///
	void destruct(const context& ctx);
};

}
}

#include <cgv/config/lib_end.h>
