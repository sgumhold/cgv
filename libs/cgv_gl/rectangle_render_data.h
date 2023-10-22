#pragma once

#include "rectangle_renderer.h"
#include "render_data_base.h"

#include "gl/lib_begin.h"

namespace cgv {
namespace render {

template <typename ColorType = render_types::rgb>
class rectangle_render_data : public render_data_base<ColorType> {
public:
	// Repeat automatically inherited typedefs from parent class, as they can't
	// be inherited again according to C++ spec
	typedef render_types::vec2 vec2;
	typedef render_types::vec3 vec3;
	typedef render_types::quat quat;

	// Base class we're going to use virtual functions from
	typedef render_data_base<ColorType> super;

protected:
	std::vector<vec2> ext;
	std::vector<vec3> transl;
	std::vector<quat> rot;
	std::vector<vec4> tex;

	bool transfer(context& ctx, rectangle_renderer& r) {
		if(super::transfer(ctx, r)) {
			r.set_position_array(ctx, this->pos);
			if(ext.size() == this->size())
				r.set_extent_array(ctx, ext);
			if(transl.size() == this->size())
				r.set_translation_array(ctx, transl);
			if(rot.size() == this->size())
				r.set_rotation_array(ctx, rot);
			if(tex.size() == this->size())
				r.set_texcoord_array(ctx, tex);
			return true;
		}
		return false;
	}

public:
	void clear() {
		super::clear();
		ext.clear();
		transl.clear();
		rot.clear();
		tex.clear();
	}

	std::vector<vec2>& ref_ext() { return ext; }
	std::vector<vec3>& ref_transl() { return transl; }
	std::vector<quat>& ref_rot() { return rot; }
	std::vector<vec4>& ref_tex() { return tex; }

	RDB_BASE_FUNC_DEF(rectangle_renderer, rectangle_render_style);

	void add(const vec3& p) {
		super::pos.push_back(p);
	}

	void add(const vec3& p, const vec2& e) {
		super::pos.push_back(p);
		ext.push_back(e);
	}

	void add(const quat& r) {
		rot.push_back(r);
	}

	void add(const vec4& tc) {
		tex.push_back(tc);
	}

	void add(const vec3& t, const quat& r) {
		transl.push_back(t);
		rot.push_back(r);
	}

	void add(const ColorType& c) {
		super::col.push_back(c);
	}

	void add(const vec3& p, const vec2& e, const ColorType& c) {
		add(p, e);
		add(c);
	}

	void add(const vec3& p, const ColorType& c) {
		add(p);
		add(c);
	}

	void fill(const vec2& e) {
		for(size_t i = ext.size(); i < super::pos.size(); ++i)
			ext.push_back(e);
	}

	void fill(const quat& r) {
		for(size_t i = rot.size(); i < super::pos.size(); ++i)
			rot.push_back(r);
	}

	void fill(const vec4& tc) {
		for(size_t i = tex.size(); i < super::pos.size(); ++i)
			tex.push_back(tc);
	}

	void fill(const ColorType& c) {
		for(size_t i = super::col.size(); i < super::pos.size(); ++i)
			super::col.push_back(c);
	}
};

}
}

#include <cgv/config/lib_end.h>
