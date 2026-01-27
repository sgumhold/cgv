#pragma once

#include <array>

#include <cgv/media/color_scale.h>

#include "shader_program.h"

#include "lib_begin.h"

namespace cgv {
namespace render {

class color_scale_adapter {
public:
	bool destruct(const context& ctx) {
		return texture_.destruct(ctx);
	}

	bool enable(context& ctx, int texture_unit = -1) {
		// Todo: Only create texture if tf has been modified after last update.
		if(create_texture(ctx)) {
			return texture_.enable(ctx, texture_unit);
		}
		return false;
	}

	bool disable(const context& ctx) {
		return texture_.disable(ctx);
	}

	void set_uniforms(context& ctx, shader_program& prog, int texture_unit = -1) {
		bool was_enabled = prog.is_enabled();
		if(!was_enabled)
			prog.enable(ctx);
		prog.set_uniform(ctx, "color_scale_texture", texture_unit);

		for(size_t i = 0; i < color_scales_.size(); ++i) {
			const auto& color_scale = color_scales_[i];
			std::string struct_name = "color_scale_mapping_options[" + std::to_string(i) + "]";
			prog.set_uniform(ctx, struct_name + ".domain", color_scale->get_domain());
			prog.set_uniform(ctx, struct_name + ".clamped", color_scale->is_clamped());
		}

		// Todo: Use uniform buffer object?

		if(!was_enabled)
			prog.disable(ctx);
	}

	bool create_texture(const context& ctx) {
		const size_t resolution = 256;

		// Todo: remove print
		std::cout << "create color scale texture" << std::endl;

		std::vector<cgv::rgba8> texture_data;

		for(const auto& color_scale : color_scales_) {
			size_t offset = texture_data.size();
			texture_data.resize(texture_data.size() + resolution);

			std::vector<rgba> colors = color_scale->quantize(resolution);
			std::transform(colors.begin(), colors.end(), texture_data.begin() + offset, [](const cgv::rgba& color) {
				return cgv::rgba8(color);
			});
		}

		cgv::data::data_view data_view(new cgv::data::data_format(resolution, color_scales_.size(), cgv::type::info::TI_UINT8, cgv::data::CF_RGBA), texture_data.data());

		// Todo: Use nearest filter for scales with discrete color ramps.
		const cgv::render::TextureFilter filter = cgv::render::TF_LINEAR;
		texture_.set_min_filter(filter);
		texture_.set_mag_filter(filter);
		return texture_.create(ctx, data_view, 0);
	}

	void set_color_scale(std::shared_ptr<const cgv::media::color_scale> color_scales) {
		color_scales_ = { color_scales };
	}

	void set_color_scales(const std::vector<std::shared_ptr<const cgv::media::color_scale>>& color_scales) {
		color_scales_ = color_scales;
	}

	//std::shared_ptr<const cgv::media::transfer_function> get_transfer_function() const {
	//	return transfer_function_;
	//}

	// Todo: Return const reference. Need to make texture enable/disable const in order to use returned texture.
	// Problem: Not so easy, because enable uses user_data from render_component which cannot be altered if texture_base is const.
	texture& get_texture() {
		return texture_;
	}

private:
	//std::shared_ptr<const cgv::media::transfer_function> transfer_function_;
	std::vector<std::shared_ptr<const cgv::media::color_scale>> color_scales_;
	texture texture_;

};

//extern CGV_API void configure_color_scales(cgv::render::context& ctx, cgv::render::shader_program& prog, const std::vector<cgv::media::color_scale*>& color_scales);

} // namespace render
} // namespace cgv

#include <cgv/config/lib_end.h>