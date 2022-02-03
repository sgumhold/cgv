#pragma once

#include <cgv/render/render_types.h>

#include "piecewise_linear_interpolator.h"

//#include "lib_begin.h"

namespace cgv {
namespace glutil{

class color_map : public cgv::render::render_types {
private:
	piecewise_linear_interpolator<rgb> color_interpolator;
	piecewise_linear_interpolator<float> opacity_interpolator;

public:
	void clear() {
		color_interpolator.clear();
		opacity_interpolator.clear();
	}

	void add_color_point(float t, rgb color) {
		// TODO: do we need to keep it in the interval [0,1]?
		// make sure t is in the interval [0,1]
		//t = cgv::math::clamp(t, 0.0f, 1.0f);
		color_interpolator.add_control_point(t, color);
	}

	void add_opacity_point(float t, float opacity) {
		opacity_interpolator.add_control_point(t, opacity);
	}

	const std::vector<decltype(color_interpolator)::control_point>& ref_color_points() { return color_interpolator.ref_control_points(); }
	const std::vector<decltype(opacity_interpolator)::control_point>& ref_opacity_points() { return opacity_interpolator.ref_control_points(); }

	rgb interpolate_color(float t) {

		return color_interpolator.interpolate(t);
	}

	float interpolate_opacity(float t) const {

		return opacity_interpolator.interpolate(t);
	}

	rgba interpolate(float t) const {

		rgb color = color_interpolator.interpolate(t);
		float opacity = opacity_interpolator.interpolate(t);

		return rgba(color.R(), color.G(), color.B(), opacity);
	}
};

}
}

//#include <cgv/config/lib_end.h>
