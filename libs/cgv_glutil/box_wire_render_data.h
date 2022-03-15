#pragma once

#include <cgv_gl/box_wire_renderer.h>
#include "render_data_base.h"

#include "lib_begin.h"

using namespace cgv::render;

namespace cgv {
namespace glutil {

template <typename ColorType = render_types::rgb>
class box_wire_render_data : public render_data_base<ColorType> {
protected:
	std::vector<vec3> ext;
	std::vector<quat> rot;

	bool transfer(context& ctx, box_wire_renderer& r) {
		if(render_data_base::transfer(ctx, r)) {
			r.set_position_array(ctx, pos);
			if(ext.size() == size())
				r.set_extent_array(ctx, ext);
			if(rot.size() == size())
				r.set_rotation_array(ctx, rot);
			return true;
		}
		return false;
	}

public:
	void clear() {
		render_data_base::clear();
		ext.clear();
		rot.clear();
	}

	std::vector<vec3>& ref_ext() { return ext; }
	std::vector<quat>& ref_rot() { return rot; }

	void early_transfer(context& ctx, box_wire_renderer& r) {
		r.enable_attribute_array_manager(ctx, aam);
		if(out_of_date) transfer(ctx, r);
		r.disable_attribute_array_manager(ctx, aam);
	}

	void render(context& ctx, box_wire_renderer& r, box_wire_render_style& s, unsigned offset = 0, int count = -1) {
		if(size() > 0) {
			r.set_render_style(s);
			r.enable_attribute_array_manager(ctx, aam);
			if(out_of_date) transfer(ctx, r);
			r.render(ctx, offset, count < 0 ? render_count() : count);
			r.disable_attribute_array_manager(ctx, aam);
		}
	}

	void add(const vec3& p) {
		pos.push_back(p);
	}

	void add(const vec3& p, const vec3& e) {
		pos.push_back(p);
		ext.push_back(e);
	}

	void add(const quat& r) {
		rot.push_back(r);
	}

	void add(const ColorType& c) {
		col.push_back(c);
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
		for(size_t i = col.size(); i < pos.size(); ++i)
			col.push_back(c);
	}
};

}
}

#include <cgv/config/lib_end.h>
