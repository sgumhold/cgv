#pragma once

#include <cgv_gl/arrow_renderer.h>
#include "render_data_base.h"

#include "lib_begin.h"

using namespace cgv::render;

namespace cgv {
namespace glutil {

template <typename ColorType = render_types::rgb>
class arrow_render_data : public render_data_base<ColorType> {
protected:
	bool direction_is_endpoint = false;
	std::vector<vec3> dir;
	
	bool transfer(context& ctx, arrow_renderer& r) {
		if(render_data_base::transfer(ctx, r)) {
			if(dir.size() == size())
				if(direction_is_endpoint)
					r.set_end_point_array(ctx, dir);
				else
					r.set_direction_array(ctx, dir);
			return true;
		}
		return false;
	}

public:
	void clear() {
		render_data_base::clear();
		dir.clear();
	}

	std::vector<vec3>& ref_dir() { return dir; }

	void early_transfer(context& ctx, arrow_renderer& r) {
		r.enable_attribute_array_manager(ctx, aam);
		if(out_of_date) transfer(ctx, r);
		r.disable_attribute_array_manager(ctx, aam);
	}

	void render(context& ctx, arrow_renderer& r, arrow_render_style& s, unsigned offset = 0, int count = -1) {
		if(size() > 0) {
			r.set_render_style(s);
			r.enable_attribute_array_manager(ctx, aam);
			if(out_of_date) transfer(ctx, r);
			r.render(ctx, offset, count < 0 ? size() : count);
			r.disable_attribute_array_manager(ctx, aam);
		}
	}

	void add(const vec3& p, const vec3& d) {
		pos.push_back(p);
		dir.push_back(d);
	}

	void add(const ColorType& c) {
		col.push_back(c);
	}

	void add(const vec3& p, const vec3& d, const ColorType& c) {
		add(p, d);
		add(c);
	}

	void fill(const ColorType& c) {
		for(size_t i = col.size(); i < pos.size(); ++i)
			col.push_back(c);
	}

	void set_direction_is_endpoint(bool flag) {
		if(direction_is_endpoint != flag) {
			direction_is_endpoint = flag;
			out_of_date = true;
		}
	}
};

}
}

#include <cgv/config/lib_end.h>
