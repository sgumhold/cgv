#pragma once

#include <cgv_gl/cone_renderer.h>
#include "render_data_base.h"

#include "lib_begin.h"

using namespace cgv::render;

namespace cgv {
namespace glutil {

template <typename ColorType = render_types::rgb>
class cone_render_data : public render_data_base {
protected:
	std::vector<float> rad;
	std::vector<ColorType> col;

	bool transfer(context& ctx, cone_renderer& r) {
		if(render_data_base::transfer(ctx, r)) {
			if(rad.size() == size())
				r.set_radius_array(ctx, rad);
			if(col.size() == size())
				r.set_color_array(ctx, col);
			return true;
		}
		return false;
	}

public:
	void clear() {
		render_data_base::clear();

		rad.clear();
		col.clear();
	}

	std::vector<float>&		ref_rad() { return rad; }
	std::vector<ColorType>&	ref_col() { return col; }

	void early_transfer(context& ctx, cone_renderer& r) {
		r.enable_attribute_array_manager(ctx, aam);
		if(out_of_date) transfer(ctx, r);
		r.disable_attribute_array_manager(ctx, aam);
	}

	void render(context& ctx, cone_renderer& r, cone_render_style& s, unsigned offset = 0, int count = -1) {
		if(size() > 0) {
			r.set_render_style(s);
			r.enable_attribute_array_manager(ctx, aam);
			if(out_of_date) transfer(ctx, r);
			r.render(ctx, offset, count < 0 ? size() : count);
			r.disable_attribute_array_manager(ctx, aam);
		}
	}

	void adds(const vec3& p) {
		pos.push_back(p);
	}

	void add(const vec3& p0, const vec3& p1) {
		pos.push_back(p0);
		pos.push_back(p1);
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
		col.push_back(c);
	}

	void add(const ColorType& c) {
		col.push_back(c);
		col.push_back(c);
	}

	void add(const ColorType& c0, const ColorType& c1) {
		col.push_back(c0);
		col.push_back(c1);
	}

	void add(const vec3& p0, const vec3& p1, const float r, const ColorType& c) {
		add(p0, p1);
		add(r);
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
