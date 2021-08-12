#pragma once

#include <cgv_gl/box_renderer.h>

#include "lib_begin.h"

using namespace cgv::render;

namespace cgv {
namespace glutil {

template <typename ColorType = render_types::rgb>
class box_render_data : public render_types {
private:
	attribute_array_manager* aam_ptr;
	bool out_of_date;

	std::vector<vec3> pos;
	std::vector<vec3> ext;
	std::vector<quat> rot;
	std::vector<ColorType> col;
	std::vector<unsigned> idx;

	void transfer(context& ctx, box_renderer& r) {
		if(pos.size() > 0) {
			r.set_position_array(ctx, pos);
			if(ext.size() == pos.size())
				r.set_extent_array(ctx, ext);
			if(rot.size() == pos.size())
				r.set_rotation_array(ctx, rot);
			if(col.size() == pos.size())
				r.set_color_array(ctx, col);
			if(idx.size() > 0)
				r.set_indices(ctx, idx);
			else
				r.remove_indices(ctx);
		}
		out_of_date = false;
	}

public:
	box_render_data(bool use_attribute_array_manager = false) {
		aam_ptr = nullptr;
		out_of_date = false;

		if(use_attribute_array_manager) {
			aam_ptr = new attribute_array_manager();
		}
	}

	~box_render_data() { clear(); }

	bool init(context& ctx) {
		if(aam_ptr)
			return aam_ptr->init(ctx);
		return true;
	}

	void destruct(context& ctx) {
		clear();

		if(aam_ptr) {
			aam_ptr->destruct(ctx);
			aam_ptr = nullptr;
		}
	}

	void clear() {
		pos.clear();
		ext.clear();
		rot.clear();
		col.clear();
		idx.clear();
		
		out_of_date = true;
	}

	void set_out_of_date() { out_of_date = true; }

	size_t size() { return pos.size(); }

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

	void add_idx(const unsigned int i) {
		idx.push_back(i);
	}

	std::vector<vec3>&		ref_pos() { return pos; }
	std::vector<vec3>&		ref_ext() { return ext; }
	std::vector<quat>&		ref_rot() { return rot; }
	std::vector<ColorType>&	ref_col() { return col; }
	std::vector<unsigned>&	ref_idx() { return idx; }

	attribute_array_manager* get_aam_ptr() { return aam_ptr; }

	void early_transfer(context& ctx, box_renderer& r) {
		if(aam_ptr) {
			r.enable_attribute_array_manager(ctx, *aam_ptr);
			if(out_of_date) transfer(ctx, r);
			r.disable_attribute_array_manager(ctx, *aam_ptr);
		} else {
			transfer(ctx, r);
		}
	}

	void render(context& ctx, box_renderer& r, box_render_style& s, unsigned offset = 0, int count = -1) {
		if(size() > 0) {
			r.set_render_style(s);
			if(aam_ptr) {
				r.enable_attribute_array_manager(ctx, *aam_ptr);
				if(out_of_date) transfer(ctx, r);
				r.render(ctx, offset, count < 0 ? size() : count);
				r.disable_attribute_array_manager(ctx, *aam_ptr);
			} else {
				transfer(ctx, r);
				r.render(ctx, offset, count < 0 ? size() : count);
			}
		}
	}
};

}
}

#include <cgv/config/lib_end.h>
