#include "color_scale_adapter.h"

namespace cgv {
namespace render {

bool color_scale_adapter::init(const context& ctx) {
	return uniform_buffer_.create(ctx, 2);
}

bool color_scale_adapter::destruct(const context& ctx) {
	uniform_buffer_.destruct(ctx);
	return texture_.destruct(ctx);
}

bool color_scale_adapter::enable(const context& ctx, int texture_unit) {
	create_texture(ctx);

	// Todo: Allow to set uniform block binding
	// Todo: Update uniform buffer here if color scales changed.

	//_uniform_buffer.replace(ctx, uniforms);
	uniform_buffer_.bind(ctx, 0);

	return texture_.enable(ctx, texture_unit);
}

bool color_scale_adapter::disable(const context& ctx) {
	uniform_buffer_.unbind(ctx, 0);
	return texture_.disable(ctx);
}

void color_scale_adapter::set_uniforms_in_program(context& ctx, shader_program& prog, int texture_unit) {
	bool was_enabled = prog.is_enabled();
	if(!was_enabled)
		prog.enable(ctx);
	prog.set_uniform(ctx, "color_scale_texture", texture_unit);
	prog.set_uniform_block_binding(ctx, "color_scale_argument_block", 0);

	for(size_t i = 0; i < color_scales_.size(); ++i)
		uniforms_[i] = color_scales_[i]->get_arguments();

	uniform_buffer_.resize_or_replace(ctx, uniforms_);

	if(!was_enabled)
		prog.disable(ctx);
}

void color_scale_adapter::set_color_scale(std::shared_ptr<const device_color_scale> color_scale) {
	if(!color_scales_.size() == 1 || color_scales_.front() != color_scale) {
		auto g = cgv::data::time_point::min();
		build_time_.reset();
		color_scales_ = { color_scale };
		uniforms_ = std::vector<device_color_scale_arguments>(1);
	}
}

void color_scale_adapter::set_color_scales(const std::vector<std::shared_ptr<const device_color_scale>>& color_scales) {
	if(color_scales_ != color_scales) {
		build_time_.reset();
		color_scales_ = color_scales;
		uniforms_ = std::vector<device_color_scale_arguments>(color_scales.size());
	}
}

// Todo: Return const reference. Need to make texture enable/disable const in order to use returned texture.
// Problem: Not so easy, because enable uses user_data from render_component which cannot be altered if texture_base is const.
texture& color_scale_adapter::get_texture(const context& ctx) {
	create_texture(ctx);
	return texture_;
}

cgv::data::time_point color_scale_adapter::get_latest_color_scale_modification_time() const {
	cgv::data::time_point time;
	for(const auto& color_scale : color_scales_)
		time = std::max(time, color_scale->get_modified_time());
	return time;
}

bool color_scale_adapter::create_texture(const context& ctx) {
	if(color_scales_.empty())
		return false;

	if(!build_time_.is_valid() || get_latest_color_scale_modification_time() > build_time_.get_modified_time()) {
		const size_t texture_width = 256;

		std::vector<cgv::rgba8> texture_data;

		for(const auto& color_scale : color_scales_) {
			size_t offset = texture_data.size();
			texture_data.resize(texture_data.size() + texture_width);
			std::vector<rgba> colors = color_scale->get_texture_data(texture_width);
			std::transform(colors.begin(), colors.end(), texture_data.begin() + offset, [](const cgv::rgba& color) {
				return cgv::rgba8(color);
			});
		}

		cgv::data::data_format data_format(texture_width, color_scales_.size(), cgv::type::info::TI_UINT8, cgv::data::CF_RGBA);
		cgv::data::data_view data_view(&data_format, texture_data.data());

		const cgv::render::TextureFilter filter = cgv::render::TF_LINEAR;
		texture_.set_min_filter(filter);
		texture_.set_mag_filter(filter);

		build_time_.modified();
		return texture_.create(ctx, data_view, 0);
	}

	return true;
}

} // namespace render
} // namespace cgv
