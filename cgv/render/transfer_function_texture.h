#pragma once

#include "texture.h"

#include <cgv/media/transfer_function.h>

namespace cgv {
namespace render {

class transfer_function_texture {
public:
	cgv::media::transfer_function transfer_function;

	bool create(const context& ctx) {
		return create_texture(ctx);
	}

	bool destruct(const context& ctx) {
		return texture_.destruct(ctx);
	}

	bool enable(const context& ctx, int texture_unit = -1) {
		// Todo: Only create texture if tf has been modified after last update.
		if(create_texture(ctx)) {
			// Todo: Set uniforms (in this function or a separate one?; need program)
			return texture_.enable(ctx, texture_unit);
		}
		return false;
	}

	bool disable(const context& ctx) {
		return texture_.disable(ctx);
	}

	// Todo: Return const reference. Need to make texture enable/disable const in order to use returned texture.
	// Problem: Not so easy, because enable uses user_data from render_component which cannot be altered if texture_base is const.
	texture& get() {
		return texture_;
	}

private:
	bool create_texture(const context& ctx) {
		// Todo: Get from tf or member.
		const size_t resolution = 256;

		std::cout << "create" << std::endl;

		std::vector<rgba> colors = transfer_function.quantize_value(resolution);

		std::vector<cgv::rgba8> colors_8bit;
		colors_8bit.reserve(resolution);
		std::transform(colors.begin(), colors.end(), std::back_inserter(colors_8bit), [](const cgv::rgba& color) {
			return cgv::rgba8(color);
		});

		cgv::data::data_view data_view(new cgv::data::data_format(resolution, 1, cgv::type::info::TI_UINT8, cgv::data::CF_RGBA), colors_8bit.data());

		// Todo: Use nearest filer for scales with discrete color ramps.
		const cgv::render::TextureFilter filter = cgv::render::TF_LINEAR;

		texture_.set_min_filter(filter);
		texture_.set_mag_filter(filter);
		return texture_.create(ctx, data_view, 0);
	}

	texture texture_;
};

} // namespace render
} // namespace cgv
