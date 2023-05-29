#pragma once

#include "point_renderer.h"
#include "render_data_base.h"

#include "gl/lib_begin.h"

namespace cgv {
namespace render {

template <typename ColorType = render_types::rgb>
class point_render_data : public render_data_base<ColorType> {
public:
	// Repeat automatically inherited typedefs from parent class, as they can't
	// be inherited again according to C++ spec
	typedef render_types::vec3 vec3;

	// Base class we're going to use virtual functions from
	typedef render_data_base<ColorType> super;

protected:
	std::vector<float> sze;

	bool transfer(context& ctx, point_renderer& r) {
		if(super::transfer(ctx, r)) {
			if(sze.size() == this->size())
				r.set_point_size_array(ctx, sze);
			return true;
		}
		return false;
	}

public:
	void clear() {
		super::clear();
		sze.clear();
	}

	std::vector<float>& ref_sze() { return sze; }

	RDB_BASE_FUNC_DEF(point_renderer, point_render_style);

	void add(const vec3& p) {
		this->pos.push_back(p);
	}

	void add(const float s) {
		sze.push_back(s);
	}

	void add(const ColorType& c) {
		this->col.push_back(c);
	}

	void add(const vec3& p, const float s, const ColorType& c) {
		add(p);
		add(s);
		add(c);
	}

	void add(const vec3& p, const float s) {
		add(p);
		add(s);
	}

	void add(const vec3& p, const ColorType& c) {
		add(p);
		add(c);
	}

	void fill(const float s) {
		for(size_t i = sze.size(); i < this->pos.size(); ++i)
			sze.push_back(s);
	}

	void fill(const ColorType& c) {
		for(size_t i = this->col.size(); i < this->pos.size(); ++i)
			this->col.push_back(c);
	}
};

}
}

#include <cgv/config/lib_end.h>
