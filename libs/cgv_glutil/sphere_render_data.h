#pragma once

#include <cgv_gl/sphere_renderer.h>
#include "render_data_base.h"

#include "lib_begin.h"

using namespace cgv::render;

namespace cgv {
namespace glutil {

template <typename ColorType = render_types::rgb>
class sphere_render_data : public render_data_base<ColorType> {
protected:
	std::vector<float> rad;

	bool transfer(context& ctx, sphere_renderer& r) {
		if(render_data_base::transfer(ctx, r)) {
			if(rad.size() == size())
				r.set_radius_array(ctx, rad);
			return true;
		}
		return false;
	}

public:
	void clear() {
		render_data_base::clear();
		rad.clear();
	}

	std::vector<float>& ref_rad() { return rad; }

	void early_transfer(context& ctx, sphere_renderer& r) {
		r.enable_attribute_array_manager(ctx, aam);
		if(out_of_date) transfer(ctx, r);
		r.disable_attribute_array_manager(ctx, aam);
	}

	void render(context& ctx, sphere_renderer& r, sphere_render_style& s, unsigned offset = 0, int count = -1) {
		if(size() > 0) {
			r.set_render_style(s);
			r.enable_attribute_array_manager(ctx, aam);
			if(out_of_date) transfer(ctx, r);
			r.render(ctx, offset, count < 0 ? size() : count);
			r.disable_attribute_array_manager(ctx, aam);
		}
	}

	void add(const vec3& p) {
		pos.push_back(p);
	}

	void add(const float r) {
		rad.push_back(r);
	}

	void add(const ColorType& c) {
		col.push_back(c);
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
		for(size_t i = rad.size(); i < pos.size(); ++i)
			rad.push_back(r);
	}

	void fill(const ColorType& c) {
		for(size_t i = col.size(); i < pos.size(); ++i)
			col.push_back(c);
	}
};

}
}

#include <cgv/config/lib_end.h>
