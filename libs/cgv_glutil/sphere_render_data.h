#pragma once

#include <cgv_gl/sphere_renderer.h>
#include "render_data_base.h"

#include "lib_begin.h"

using namespace cgv::render;

namespace cgv {
namespace glutil {

template <typename ColorType = render_types::rgb>
class sphere_render_data : public render_data_base<ColorType> {
public:
	// Repeat automatically inherited typedefs from parent class, as they can't
	// be inherited again according to C++ spec
	typedef render_types::vec3 vec3;

	// Base class we're going to use virtual functions from
	typedef render_data_base<ColorType> super;

protected:
	std::vector<float> rad;

	bool transfer(context& ctx, sphere_renderer& r) {
		if(super::transfer(ctx, r)) {
			if(rad.size() == this->size())
				r.set_radius_array(ctx, rad);
			return true;
		}
		return false;
	}

public:
	void clear() {
		super::clear();
		rad.clear();
	}

	std::vector<float>& ref_rad() { return rad; }

	RDB_BASE_FUNC_DEF(sphere_renderer, sphere_render_style);

	void add(const vec3& p) {
		this->pos.push_back(p);
	}

	void add(const float r) {
		rad.push_back(r);
	}

	void add(const ColorType& c) {
		this->col.push_back(c);
	}

	void add(const vec3& p, const float r, const ColorType& c) {
		add(p);
		add(r);
		add(c);
	}

	void add(const vec3& p, const float r) {
		add(p);
		add(r);
	}

	void add(const vec3& p, const ColorType& c) {
		add(p);
		add(c);
	}

	void fill(const float& r) {
		for(size_t i = rad.size(); i < this->pos.size(); ++i)
			rad.push_back(r);
	}

	void fill(const ColorType& c) {
		for(size_t i = this->col.size(); i < this->pos.size(); ++i)
			this->col.push_back(c);
	}
};

}
}

#include <cgv/config/lib_end.h>
