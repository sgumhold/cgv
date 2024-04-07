#pragma once

#include <memory>

#include <cgv/math/piecewise_linear_interpolator.h>
#include <cgv/math/piecewise_nearest_interpolator.h>
#include <cgv/render/context.h>
#include <cgv/render/texture.h>

namespace cgv {
namespace render {

class color_map {
protected:
	typedef cgv::math::control_point_container<rgb>::control_point color_control_point_type;
	typedef cgv::math::control_point_container<float>::control_point opacity_control_point_type;

	cgv::math::control_point_container<rgb> color_points;
	cgv::math::control_point_container<float> opacity_points;

	std::shared_ptr<cgv::math::interpolator<rgb>> color_interpolator_ptr = nullptr;
	std::shared_ptr<cgv::math::interpolator<float>> opacity_interpolator_ptr = nullptr;

	/// resolution of the sampled color map; mostly used when generating textures from color maps
	unsigned resolution = 256u;
	/// whether to use interpolation between the samples or just take the nearest one
	bool use_interpolation = true;
	
	void construct_interpolators() {
		if(use_interpolation) {
			color_interpolator_ptr = std::make_shared<cgv::math::piecewise_linear_interpolator<rgb>>();
			opacity_interpolator_ptr = std::make_shared<cgv::math::piecewise_linear_interpolator<float>>();
		} else {
			color_interpolator_ptr = std::make_shared<cgv::math::piecewise_nearest_interpolator<rgb>>();
			opacity_interpolator_ptr = std::make_shared<cgv::math::piecewise_nearest_interpolator<float>>();
		}
	}

public:
	color_map() {
		construct_interpolators();
	}

	virtual ~color_map() {}

	virtual bool has_texture_support() const {
		return false;
	}

	void clear() {
		color_points.clear();
		opacity_points.clear();
	}

	void clear_color_points() {
		color_points.clear();
	}

	void clear_opacity_points() {
		opacity_points.clear();
	}

	bool empty() const {
		return color_points.empty() && opacity_points.empty();
	}

	unsigned get_resolution() const { return resolution; }

	void set_resolution(unsigned resolution) {
		this->resolution = std::max(resolution, 2u);
	}

	bool is_interpolation_enabled() const { return use_interpolation; }

	void enable_interpolation(bool enabled) {
		use_interpolation = enabled;
		construct_interpolators();
	}

	void flip() {

		cgv::math::control_point_container<rgb> flipped_color_points;
		cgv::math::control_point_container<float> flipped_opacity_points;

		for(const color_control_point_type& color_point : color_points)
			flipped_color_points.push_back(1.0f - color_point.first, color_point.second);

		for(const opacity_control_point_type& opacity_point : opacity_points)
			flipped_opacity_points.push_back(1.0f - opacity_point.first, opacity_point.second);

		color_points = flipped_color_points;
		opacity_points = flipped_opacity_points;
	}

	void apply_gamma(float gamma) {

		cgv::math::control_point_container<rgb> corrected_color_points;

		for(const color_control_point_type& color_point : color_points)
			corrected_color_points.push_back(color_point.first, cgv::media::pow(color_point.second, gamma));

		color_points = corrected_color_points;
	}

	void add_color_point(float t, rgb color) {
		t = cgv::math::clamp(t, 0.0f, 1.0f);
		color_points.push_back(t, color);
	}

	void add_opacity_point(float t, float opacity) {
		t = cgv::math::clamp(t, 0.0f, 1.0f);
		opacity_points.push_back(t, opacity);
	}

	const std::vector<color_control_point_type>& ref_color_points() const { return color_points.ref_points(); }
	const std::vector<opacity_control_point_type>& ref_opacity_points() const { return opacity_points.ref_points(); }

	rgb interpolate_color(float t) const {

		return color_interpolator_ptr->interpolate(color_points, t);
	}

	std::vector<rgb> interpolate_color(size_t n) const {

		return color_interpolator_ptr->interpolate(color_points, n);
	}

	float interpolate_opacity(float t) const {

		return opacity_interpolator_ptr->interpolate(opacity_points, t);
	}

	std::vector<float> interpolate_opacity(size_t n) const {

		return opacity_interpolator_ptr->interpolate(opacity_points, n);
	}

	rgba interpolate(float t) const {

		rgb color = interpolate_color(t);
		float opacity = interpolate_opacity(t);

		return rgba(color.R(), color.G(), color.B(), opacity);
	}

	std::vector<rgba> interpolate(size_t n) const {

		const std::vector<rgb>& colors = interpolate_color(n);
		const std::vector<float>& opacities = interpolate_opacity(n);

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
	/// whether to use linear or nearest neighbour texture filtering 
	bool use_linear_filtering = true;
	texture tex;
	
	TextureFilter get_texture_filter() {
		return use_linear_filtering ? TF_LINEAR : TF_NEAREST;
	}

	void setup_texture(bool use_opacity) {
		std::string format = use_opacity ? "uint8[R,G,B,A]" : "uint8[R,G,B]";
		TextureFilter filter = get_texture_filter();
		tex = texture(format, filter, filter);
	}

	void generate_rgb_texture(context& ctx) {
		std::vector<rgb> data = interpolate_color(static_cast<size_t>(resolution));
		
		std::vector<uint8_t> data_8(3 * data.size());
		for(size_t i = 0; i < data.size(); ++i) {
			rgb col = data[i];
			data_8[3 * i + 0] = static_cast<uint8_t>(255.0f * col.R());
			data_8[3 * i + 1] = static_cast<uint8_t>(255.0f * col.G());
			data_8[3 * i + 2] = static_cast<uint8_t>(255.0f * col.B());
		}

		cgv::data::data_view dv = cgv::data::data_view(new cgv::data::data_format(resolution, TI_UINT8, cgv::data::CF_RGB), data_8.data());

		unsigned width = (unsigned)tex.get_width();

		bool replaced = false;
		if(tex.is_created() && width == resolution && tex.get_nr_components() == 3) {
			TextureFilter filter = get_texture_filter();
			tex.set_min_filter(filter);
			tex.set_mag_filter(filter);
			replaced = tex.replace(ctx, 0, dv);
		}

		if(!replaced) {
			tex.destruct(ctx);
			setup_texture(false);
			tex.create(ctx, dv, 0);
		}
	}

	void generate_rgba_texture(context& ctx) {
		std::vector<rgba> data = interpolate(static_cast<size_t>(resolution));

		std::vector<uint8_t> data_8(4 * data.size());
		for(size_t i = 0; i < data.size(); ++i) {
			rgba col = data[i];
			data_8[4 * i + 0] = static_cast<uint8_t>(255.0f * col.R());
			data_8[4 * i + 1] = static_cast<uint8_t>(255.0f * col.G());
			data_8[4 * i + 2] = static_cast<uint8_t>(255.0f * col.B());
			data_8[4 * i + 3] = static_cast<uint8_t>(255.0f * col.alpha());
		}

		cgv::data::data_view dv = cgv::data::data_view(new cgv::data::data_format(resolution, TI_UINT8, cgv::data::CF_RGBA), data_8.data());

		unsigned width = (unsigned)tex.get_width();

		bool replaced = false;
		if(tex.is_created() && width == resolution && tex.get_nr_components() == 4) {
			TextureFilter filter = get_texture_filter();
			tex.set_min_filter(filter);
			tex.set_mag_filter(filter);
			replaced = tex.replace(ctx, 0, dv);
		}

		if(!replaced) {
			tex.destruct(ctx);
			setup_texture(true);
			tex.create(ctx, dv, 0);
		}
	}

public:
	gl_color_map() : color_map() {}

	gl_color_map(unsigned resolution) : gl_color_map() {
		resolution = std::max(resolution, 2u);
	}
	
	gl_color_map(const color_map& cm) : gl_color_map() {
		resolution = cm.get_resolution();
		use_interpolation = cm.is_interpolation_enabled();
		
		construct_interpolators();

		for(auto& p : cm.ref_color_points())
			add_color_point(p.first, p.second);
		for(auto& p : cm.ref_opacity_points())
			add_opacity_point(p.first, p.second);
	}

	virtual ~gl_color_map() {
		clear();
	}

	virtual bool has_texture_support() const {
		return true;
	}

	void clear() {
		color_map::clear();
	}

	bool destruct(context& ctx) {
		if(tex.is_created())
			return tex.destruct(ctx);
		return true;
	}

	bool is_linear_filtering_enabled() const { return use_linear_filtering; }

	void enable_linear_filtering(bool enabled) {
		use_linear_filtering = enabled;
		construct_interpolators();
	}

	bool init(context& ctx) {
		std::vector<uint8_t> data(3 * resolution);

		tex.destruct(ctx);
		cgv::data::data_view dv = cgv::data::data_view(new cgv::data::data_format(resolution, TI_UINT8, cgv::data::CF_RGB), data.data());
		setup_texture(false);
		return tex.create(ctx, dv, 0);
	}

	void generate_texture(context& ctx) {

		if(ref_opacity_points().size() > 0)
			generate_rgba_texture(ctx);
		else
			generate_rgb_texture(ctx);
	}

	texture& ref_texture() { return tex; }
};

}
}
