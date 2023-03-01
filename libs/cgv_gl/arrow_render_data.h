#pragma once

#include "arrow_renderer.h"
#include "render_data_base.h"

#include "gl/lib_begin.h"

namespace cgv {
namespace render {

template <typename ColorType = render_types::rgb>
class arrow_render_data : public render_data_base<ColorType> {
public:
	// Repeat automatically inherited typedefs from parent class, as they can't
	// be inherited again according to C++ spec
	typedef render_types::vec3 vec3;

	// Base class we're going to use virtual functions from
	typedef render_data_base<ColorType> super;

protected:
	bool direction_is_endpoint = false;
	std::vector<vec3> dir;
	
	bool transfer(context& ctx, arrow_renderer& r) {
		if(super::transfer(ctx, r)) {
			if(dir.size() == this->size()) {
				if(direction_is_endpoint)
					r.set_end_point_array(ctx, dir);
				else
					r.set_direction_array(ctx, dir);
			}
			return true;
		}
		return false;
	}

public:
	void clear() {
		super::clear();
		dir.clear();
	}

	std::vector<vec3>& ref_dir() { return dir; }

	RDB_BASE_FUNC_DEF(arrow_renderer, arrow_render_style);

	void add(const vec3& p, const vec3& d) {
		this->pos.push_back(p);
		dir.push_back(d);
	}

	void add(const ColorType& c) {
		this->col.push_back(c);
	}

	void add(const vec3& p, const vec3& d, const ColorType& c) {
		add(p, d);
		add(c);
	}

	void fill(const ColorType& c) {
		for(size_t i = this->col.size(); i < this->pos.size(); ++i)
			this->col.push_back(c);
	}

	void set_direction_is_endpoint(bool flag) {
		if(direction_is_endpoint != flag) {
			direction_is_endpoint = flag;
			this->out_of_date = true;
		}
	}
};

}
}

#include <cgv/config/lib_end.h>
