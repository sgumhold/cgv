#pragma once

#include <cgv/render/context.h>
#include <cgv/render/render_types.h>
#include <cgv/render/shader_program.h>
#include <cgv/render/attribute_array_binding.h>
#include <cgv_gl/attribute_array_manager.h>

#include "lib_begin.h"

// defines are prefixed with GRD (generic render data) to prevent conflicts with other libraries
#define GRD_EXPAND(x) x

#define GRD_VAR_TO_STRING_(s) #s
#define GRD_VAR_TO_STRING(s) GRD_VAR_TO_STRING_(s)

#define GRD_DECL_VEC_MEMBER(t, i) std::vector<t> i;
#define GRD_DECL_VEC_MEMBER_CONST_REF(t, i) const std::vector<t>& ref_##i() const { return i; }
#define GRD_CALL_CLEAR_FUNC(t, i) i.clear();
#define GRD_CALL_SIZE_FUNC(t, i) i.size();
#define GRD_CALL_PUSH_BACK_FUNC(t, i) this->i.push_back(i);
#define GRD_SET_ATTRIB_ARRAY(t, i) success &= set_attribute_array(ctx, prog, GRD_VAR_TO_STRING(i), i);
#define GRD_DEF_CONST_REF_PARAM(t, i) const t& i

#define GRD_SEP_NULL(x)
#define GRD_SEP_COMMA(x) ,

#define GRD_APPLY_FUNC1(f, s, t0, i0) f(t0, i0)

#define GRD_APPLY_FUNC2(f, s, t0, i0, t1, i1)\
	GRD_APPLY_FUNC1(f, s, t0, i0) s(0)\
	GRD_APPLY_FUNC1(f, s, t1, i1)

#define GRD_APPLY_FUNC3(f, s, t0, i0, t1, i1, t2, i2)\
	GRD_APPLY_FUNC1(f, s, t0, i0) s(0)\
	GRD_APPLY_FUNC2(f, s, t1, i1, t2, i2)

#define GRD_APPLY_FUNC4(f, s, t0, i0, t1, i1, t2, i2, t3, i3)\
	GRD_APPLY_FUNC1(f, s, t0, i0) s(0)\
	GRD_APPLY_FUNC3(f, s, t1, i1, t2, i2, t3, i3)

#define GRD_APPLY_FUNC5(f, s, t0, i0, t1, i1, t2, i2, t3, i3, t4, i4)\
	GRD_APPLY_FUNC1(f, s, t0, i0) s(0)\
	GRD_APPLY_FUNC4(f, s, t1, i1, t2, i2, t3, i3, t4, i4)

#define GRD_APPLY_FUNC6(f, s, t0, i0, t1, i1, t2, i2, t3, i3, t4, i4, t5, i5)\
	GRD_APPLY_FUNC1(f, s, t0, i0) s(0)\
	GRD_APPLY_FUNC5(f, s, t1, i1, t2, i2, t3, i3, t4, i4, t5, i5)

#define GRD_APPLY_FUNC7(f, s, t0, i0, t1, i1, t2, i2, t3, i3, t4, i4, t5, i5, t6, i6)\
	GRD_APPLY_FUNC1(f, s, t0, i0) s(0)\
	GRD_APPLY_FUNC6(f, s, t1, i1, t2, i2, t3, i3, t4, i4, t5, i5, t6, i6)

#define GRD_APPLY_FUNC8(f, s, t0, i0, t1, i1, t2, i2, t3, i3, t4, i4, t5, i5, t6, i6, t7, i7)\
	GRD_APPLY_FUNC1(f, s, t0, i0) s(0)\
	GRD_APPLY_FUNC7(f, s, t1, i1, t2, i2, t3, i3, t4, i4, t5, i5, t6, i6, t7, i7)

#define GRD_APPLY_FUNC9(f, s, t0, i0, t1, i1, t2, i2, t3, i3, t4, i4, t5, i5, t6, i6, t7, i7, t8, i8)\
	GRD_APPLY_FUNC1(f, s, t0, i0) s(0)\
	GRD_APPLY_FUNC8(f, s, t1, i1, t2, i2, t3, i3, t4, i4, t5, i5, t6, i6, t7, i7, t8, i8)

#define GRD_APPLY_FUNC10(f, s, t0, i0, t1, i1, t2, i2, t3, i3, t4, i4, t5, i5, t6, i6, t7, i7, t8, i8, t9, i9)\
	GRD_APPLY_FUNC1(f, s, t0, i0) s(0)\
	GRD_APPLY_FUNC9(f, s, t1, i1, t2, i2, t3, i3, t4, i4, t5, i5, t6, i6, t7, i7, t8, i8, t9, i9)

#define GRD_APPLY_FUNC_N(n, f, s, ...) GRD_EXPAND( GRD_APPLY_FUNC##n(f, s, __VA_ARGS__) )

#define GRD_GET_FIRST_PAIR_(f, t, i, ...) f(t, i)
#define GRD_GET_FIRST_PAIR(f, ...) GRD_EXPAND( GRD_GET_FIRST_PAIR_(f, __VA_ARGS__) )

namespace cgv {

/// forward declaration to give generic renderer access to protected members
namespace g2d {
class generic_2d_renderer;
}

namespace glutil {

/// forward declaration to give generic renderer access to protected members
class generic_renderer;

class generic_render_data : public cgv::render::render_types {
	friend class generic_renderer;
	friend class cgv::g2d::generic_2d_renderer;
private:
	cgv::render::attribute_array_manager aam;
	
protected:
	bool state_out_of_date = true;
	std::vector<unsigned> idx;

	template <typename T>
	bool set_attribute_array(const cgv::render::context& ctx, const cgv::render::shader_program& prog, const std::string& name, const T& array) {
		int loc = prog.get_attribute_location(ctx, name);
		if(loc > -1)
			return aam.set_attribute_array(ctx, loc, array);
		return false;
	}

	bool has_indices() const {
		return aam.has_index_buffer();
	}

	bool set_indices(const cgv::render::context& ctx) {
		return aam.set_indices(ctx, idx);
	}

	void remove_indices(const cgv::render::context& ctx) {
		return aam.remove_indices(ctx);
	}

	virtual bool transfer(cgv::render::context& ctx, cgv::render::shader_program& prog) = 0;

	bool enable(cgv::render::context& ctx, cgv::render::shader_program& prog) {
		if(!aam.is_created())
			aam.init(ctx);
		bool res = true;
		if(state_out_of_date)
			transfer(ctx, prog);
		state_out_of_date = false;
		if(!res)
			return false;
		return aam.enable(ctx);
	}

	bool disable(cgv::render::context& ctx) {
		return aam.disable(ctx);
	}

public:
	void destruct(const cgv::render::context& ctx) {
		aam.destruct(ctx);
	}

	bool init(cgv::render::context& ctx) {
		return aam.init(ctx);
	}

	void add_idx(const unsigned int i) { idx.push_back(i); }

	std::vector<unsigned>& ref_idx() { return idx; }
	const std::vector<unsigned>& ref_idx() const { return idx; }

	void set_out_of_date() {
		state_out_of_date = true;
	}

	virtual size_t get_render_count() const = 0;
};

}
}

/** This macro provides a shortcut to define a class that inherits from the abstract generic render data base class,
	that can be used with a generic renderer.
	Arguments taken are (class name, attribute count, [type, name], [...])
	Omit square brackets when caling the macro, see example.
	Up to 8 attributes are supported. Attribute count must match the number of [type, name] pairs.

	The macro generates:
		> a class inheriting from generic_render_data
		> a public member for each vertex attribute in the form: std::vector<type> name
		> a size method that returns the size of the vector for the first specified vertex attribute
		> a clear method to clear all vertex attribute data
		> an add method to simultaneously add data to all vertex attributes
		> implementation of the virtual methods from generic_render_data (size and transfer)

	====== Example usage ======

	Use the macro like this in your own implementation:
	DEFINE_GENERIC_RENDER_DATA_CLASS(example_geometry, 2, vec2, position, rgb, color);

	This will create a definition for a class named example_geometry, which contains
	two vectors of vertex attributes:
		position of type vec2
		color of type rgb

	The fully expanded code for the example macro looks like this:
	class example_geometry : public generic_render_data {
	protected:
		bool transfer(context& ctx, shader_program& prog) {
			bool success = true;
			if(get_render_count() == 0) return false;\
			success &= set_attribute_array(ctx, prog, "position", positions);
			success &= set_attribute_array(ctx, prog, "color", colors);
			return success;
		}
	public:
		std::vector<vec2> positions;
		std::vector<rgb> colors;

		size_t get_render_count() const {
			if(idx.empty() return positions.size();
			else return idx.size();
		};

		void clear() {
			positions.clear();
			colors.clear();
			state_out_of_date = true;
		}

		void add(const vec2& pos, const rgb& col) {
			positions.push_back(pos);
			colors.push_back(col);
		}
	};
*/

#define DEFINE_GENERIC_RENDER_DATA_CLASS(name, attrib_count, ...)\
class name : public cgv::glutil::generic_render_data {\
protected:\
	bool transfer(cgv::render::context& ctx, cgv::render::shader_program& prog) {\
		bool success = true;\
		if(get_render_count() == 0) return false;\
		if(idx.size() > 0) set_indices(ctx);\
		else remove_indices(ctx);\
		GRD_APPLY_FUNC_N(attrib_count, GRD_SET_ATTRIB_ARRAY, GRD_SEP_NULL, __VA_ARGS__)\
		return success;\
	}\
public:\
	GRD_APPLY_FUNC_N(attrib_count, GRD_DECL_VEC_MEMBER, GRD_SEP_NULL, __VA_ARGS__)\
	GRD_APPLY_FUNC_N(attrib_count, GRD_DECL_VEC_MEMBER_CONST_REF, GRD_SEP_NULL, __VA_ARGS__)\
	size_t get_render_count() const {\
		if(idx.empty()) return GRD_GET_FIRST_PAIR(GRD_CALL_SIZE_FUNC, __VA_ARGS__)\
		else return idx.size();\
	}\
	void clear() {\
		idx.clear();\
		GRD_APPLY_FUNC_N(attrib_count, GRD_CALL_CLEAR_FUNC, GRD_SEP_NULL, __VA_ARGS__)\
		state_out_of_date = true;\
	}\
	void add(GRD_APPLY_FUNC_N(attrib_count, GRD_DEF_CONST_REF_PARAM, GRD_SEP_COMMA, __VA_ARGS__)) {\
		GRD_APPLY_FUNC_N(attrib_count, GRD_CALL_PUSH_BACK_FUNC, GRD_SEP_NULL, __VA_ARGS__)\
	}\
};

#include <cgv/config/lib_end.h>
