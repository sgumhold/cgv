#pragma once

#include "box_renderer.h"
#include "render_data_base.h"

#include "gl/lib_begin.h"

namespace cgv {
namespace render {

template <typename ColorType = render_types::rgb>
class box_render_data : public render_data_base<ColorType> {
public:
	// Repeat automatically inherited typedefs from parent class, as they can't
	// be inherited again according to C++ spec
	typedef render_types::vec3 vec3;
	typedef render_types::quat quat;

	// Base class we're going to use virtual functions from
	typedef render_data_base<ColorType> super;

protected:
	std::vector<vec3> ext;
	std::vector<quat> rot;

	bool transfer(context& ctx, box_renderer& r) {
		if(super::transfer(ctx, r)) {
			r.set_position_array(ctx, this->pos);
			if(ext.size() == this->size())
				r.set_extent_array(ctx, ext);
			if(rot.size() == this->size())
				r.set_rotation_array(ctx, rot);
			return true;
		}
		return false;
	}

public:
	void clear() {
		super::clear();
		ext.clear();
		rot.clear();
	}

	std::vector<vec3>& ref_ext() { return ext; }
	std::vector<quat>& ref_rot() { return rot; }

	RDB_BASE_FUNC_DEF(box_renderer, box_render_style);

	void add(const vec3& p) {
		this->pos.push_back(p);
	}

	void add(const vec3& p, const vec3& e) {
		this->pos.push_back(p);
		ext.push_back(e);
	}

	void add(const quat& r) {
		rot.push_back(r);
	}

	void add(const ColorType& c) {
		this->col.push_back(c);
	}

	void add(const vec3& p, const vec3& e, const ColorType& c) {
		add(p, e);
		add(c);
	}

	void add(const vec3& p, const ColorType& c) {
		add(p);
		add(c);
	}

	void fill(const vec3& e) {
		for(size_t i = ext.size(); i < ext.size(); ++i)
			ext.push_back(e);
	}

	void fill(const quat& r) {
		for(size_t i = rot.size(); i < rot.size(); ++i)
			rot.push_back(r);
	}

	void fill(const ColorType& c) {
		for(size_t i = this->col.size(); i < this->pos.size(); ++i)
			this->col.push_back(c);
	}
};

}
}

#include <cgv/config/lib_end.h>
