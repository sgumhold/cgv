#pragma once

#include "cone_renderer.h"
#include "render_data_base.h"

#include "gl/lib_begin.h"

namespace cgv {
namespace render {

template <typename ColorType = render_types::rgb>
class cone_render_data : public render_data_base<ColorType> {
public:
	// Repeat automatically inherited typedefs from parent class, as they can't
	// be inherited again according to C++ spec
	typedef render_types::vec3 vec3;

	// Base class we're going to use virtual functions from
	typedef render_data_base<ColorType> super;

protected:
	std::vector<float> rad;

	bool transfer(context& ctx, cone_renderer& r) {
		if(super::transfer(ctx, r)) {
			if(rad.size() == super::size())
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

	RDB_BASE_FUNC_DEF(cone_renderer, cone_render_style);

	void adds(const vec3& p) {
		super::pos.push_back(p);
	}

	void add(const vec3& p0, const vec3& p1) {
		super::pos.push_back(p0);
		super::pos.push_back(p1);
	}

	void adds(const float r) {
		rad.push_back(r);
	}

	void add(const float r) {
		rad.push_back(r);
		rad.push_back(r);
	}

	void add(const float r0, const float r1) {
		rad.push_back(r0);
		rad.push_back(r1);
	}

	void adds(const ColorType& c) {
		super::col.push_back(c);
	}

	void add(const ColorType& c) {
		super::col.push_back(c);
		super::col.push_back(c);
	}

	void add(const ColorType& c0, const ColorType& c1) {
		super::col.push_back(c0);
		super::col.push_back(c1);
	}

	void add(const vec3& p0, const vec3& p1, const float r, const ColorType& c) {
		add(p0, p1);
		add(r);
		add(c);
	}

	void fill(const float& r) {
		for(size_t i = rad.size(); i < super::pos.size(); ++i)
			rad.push_back(r);
	}

	void fill(const ColorType& c) {
		for(size_t i = super::col.size(); i < super::pos.size(); ++i)
			super::col.push_back(c);
	}
};

}
}

#include <cgv/config/lib_end.h>
