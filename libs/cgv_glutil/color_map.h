#pragma once

#include <cgv/render/context.h>
#include <cgv/render/render_types.h>
#include <cgv/render/texture.h>

#include "piecewise_linear_interpolator.h"

//#include "lib_begin.h"

namespace cgv {
namespace glutil{

class color_map : public cgv::render::render_types {
protected:
	piecewise_linear_interpolator<rgb> color_interpolator;
	piecewise_linear_interpolator<float> opacity_interpolator;

	// TODO: make resolution adjustable by color_map_editor
	/// resolution mostly used when generating textures from color maps
	unsigned resolution = 256u;

public:
	void clear() {
		color_interpolator.clear();
		opacity_interpolator.clear();
	}

	void clear_color_points() {
		color_interpolator.clear();
	}

	void clear_opacity_points() {
		opacity_interpolator.clear();
	}

	bool empty() {
		return color_interpolator.empty() && opacity_interpolator.empty();
	}

	unsigned get_resolution() { return resolution; }

	void set_resolution(unsigned resolution) {

		this->resolution = std::max(resolution, 2u);
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

	const std::vector<decltype(color_interpolator)::control_point>& ref_color_points() const { return color_interpolator.ref_control_points(); }
	const std::vector<decltype(opacity_interpolator)::control_point>& ref_opacity_points() const { return opacity_interpolator.ref_control_points(); }

	rgb interpolate_color(float t) const {

		return color_interpolator.interpolate(t);
	}

	std::vector<rgb> interpolate_color(size_t n) const {

		return color_interpolator.interpolate(n);
	}

	float interpolate_opacity(float t) const {

		return opacity_interpolator.interpolate(t);
	}

	std::vector<float> interpolate_opacity(size_t n) const {

		return opacity_interpolator.interpolate(n);
	}

	rgba interpolate(float t) const {

		rgb color = color_interpolator.interpolate(t);
		float opacity = opacity_interpolator.interpolate(t);

		return rgba(color.R(), color.G(), color.B(), opacity);
	}

	std::vector<rgba> interpolate(size_t n) const {

		std::vector<rgb> colors = color_interpolator.interpolate(n);
		std::vector<float> opacities = opacity_interpolator.interpolate(n);

		std::vector<rgba> data(n);
		for(size_t i = 0; i < n; ++i) {
			const rgb& color = colors[i];
			data[i] = rgba(color.R(), color.G(), color.B(), opacities[i]);
		}

		return data;
	}
};

class gl_color_map : public color_map {
protected:
	cgv::render::texture tex;
	
	void generate_rgb_texture(cgv::render::context& ctx) {
		std::vector<rgb> data = interpolate_color(static_cast<size_t>(resolution));
		
		std::vector<uint8_t> data_8(3 * data.size());
		for(unsigned i = 0; i < data.size(); ++i) {
			rgba col = data[i];
			data_8[3 * i + 0] = static_cast<uint8_t>(255.0f * col.R());
			data_8[3 * i + 1] = static_cast<uint8_t>(255.0f * col.G());
			data_8[3 * i + 2] = static_cast<uint8_t>(255.0f * col.B());
		}

		cgv::data::data_view dv = cgv::data::data_view(new cgv::data::data_format(resolution, TI_UINT8, cgv::data::CF_RGB), data_8.data());

		unsigned width = tex.get_width();

		bool replaced = false;
		if(tex.is_created() && width == resolution && tex.get_nr_components() == 3)
			replaced = tex.replace(ctx, 0, dv);

		if(!replaced) {
			tex.destruct(ctx);
			tex = cgv::render::texture("uint8[R,G,B]", cgv::render::TF_LINEAR, cgv::render::TF_LINEAR);
			tex.create(ctx, dv, 0);
		}
	}

	void generate_rgba_texture(cgv::render::context& ctx) {
		std::vector<rgba> data = interpolate(static_cast<size_t>(resolution));

		std::vector<uint8_t> data_8(4 * data.size());
		for(unsigned i = 0; i < data.size(); ++i) {
			rgba col = data[i];
			data_8[4 * i + 0] = static_cast<uint8_t>(255.0f * col.R());
			data_8[4 * i + 1] = static_cast<uint8_t>(255.0f * col.G());
			data_8[4 * i + 2] = static_cast<uint8_t>(255.0f * col.B());
			data_8[4 * i + 3] = static_cast<uint8_t>(255.0f * col.alpha());
		}

		cgv::data::data_view dv = cgv::data::data_view(new cgv::data::data_format(resolution, TI_UINT8, cgv::data::CF_RGBA), data_8.data());

		unsigned width = tex.get_width();

		bool replaced = false;
		if(tex.is_created() && width == resolution && tex.get_nr_components() == 4)
			replaced = tex.replace(ctx, 0, dv);

		if(!replaced) {
			tex.destruct(ctx);
			tex = cgv::render::texture("uint8[R,G,B,A]", cgv::render::TF_LINEAR, cgv::render::TF_LINEAR);
			tex.create(ctx, dv, 0);
		}
	}

public:
	gl_color_map() {}

	gl_color_map(unsigned resolution) : gl_color_map() {
		resolution = std::max(resolution, 2u);
	}
	
	~gl_color_map() {
		clear();
	}

	void clear() {
		color_map::clear();
		tex.clear();
	}

	bool destruct(cgv::render::context& ctx) {
		if(tex.is_created())
			return tex.destruct(ctx);
		return true;
	}

	bool init(cgv::render::context& ctx) {
		std::vector<uint8_t> data(3 * resolution);

		tex.destruct(ctx);
		cgv::data::data_view dv = cgv::data::data_view(new cgv::data::data_format(resolution, TI_UINT8, cgv::data::CF_RGB), data.data());
		tex = cgv::render::texture("uint8[R,G,B]", cgv::render::TF_LINEAR, cgv::render::TF_LINEAR);
		return tex.create(ctx, dv, 0);
	}

	void generate_texture(cgv::render::context& ctx) {

		if(ref_opacity_points().size() > 0)
			generate_rgba_texture(ctx);
		else
			generate_rgb_texture(ctx);
	}

	cgv::render::texture& ref_texture() { return tex; }
};

}
}

//#include <cgv/config/lib_end.h>
