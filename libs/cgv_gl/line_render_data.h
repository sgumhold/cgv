#pragma once

#include "line_renderer.h"
#include "render_data_base.h"

#include "gl/lib_begin.h"

namespace cgv {
namespace render {

template <typename ColorType = render_types::rgb>
class line_render_data : public render_data_base<ColorType> {
public:
	// Repeat automatically inherited typedefs from parent class, as they can't
	// be inherited again according to C++ spec
	typedef render_types::vec3 vec3;

	// Base class we're going to use virtual functions from
	typedef render_data_base<ColorType> super;

protected:
	std::vector<vec3> nml;
	std::vector<float> wdth;

	bool transfer(context& ctx, line_renderer& r) {
		if(super::transfer(ctx, r)) {
			if(nml.size() == super::size())
				r.set_normal_array(ctx, nml);
			if(wdth.size() == super::size())
				r.set_line_width_array(ctx, wdth);
			return true;
		}
		return false;
	}

public:
	void clear() {
		super::clear();
		nml.clear();
		wdth.clear();
	}

	std::vector<float>& ref_nml() { return nml; }
	std::vector<float>& ref_wdth() { return wdth; }

	RDB_BASE_FUNC_DEF(line_renderer, line_render_style);

	void adds(const vec3& p) {
		super::pos.push_back(p);
	}

	void add(const vec3& p0, const vec3& p1) {
		super::pos.push_back(p0);
		super::pos.push_back(p1);
	}

	void add(const vec3& p0, const vec3& p1, const vec3& n) {
		super::pos.push_back(p0);
		super::pos.push_back(p1);
		nml.push_back(n);
	}

	void adds(const float w) {
		wdth.push_back(w);
	}

	void add(const float r) {
		wdth.push_back(r);
		wdth.push_back(r);
	}

	void add(const float w0, const float w1) {
		wdth.push_back(w0);
		wdth.push_back(w1);
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

	void add(const vec3& p0, const vec3& p1, const float w, const ColorType& c) {
		add(p0, p1);
		add(w);
		add(c);
	}

	void fill(const float w) {
		for(size_t i = wdth.size(); i < super::pos.size(); ++i)
			wdth.push_back(w);
	}

	void fill(const ColorType& c) {
		for(size_t i = super::col.size(); i < super::pos.size(); ++i)
			super::col.push_back(c);
	}
};

}
}

#include <cgv/config/lib_end.h>
