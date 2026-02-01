#pragma once

#include <vector>

#include <cgv/data/time_stamp.h>
#include <cgv/media/color_scale.h>
#include <cgv/media/transfer_function.h>

#include "shader_program.h"
#include "vertex_buffer.h"

#include "lib_begin.h"

namespace cgv {
namespace render {


// Todo: Which name prefix to choose? gl, device, gpu?

// Todo: use uniform buffer to store color scale mapping arguments

// Todo: Move uniform buffer implementation from gpgpu to render and remove this temporary local copy.

template<class T>
class uniform_buffer : public vertex_buffer {
public:
	uniform_buffer(VertexBufferUsage usage = VertexBufferUsage::VBU_STREAM_COPY) : vertex_buffer(VertexBufferType::VBT_UNIFORM, usage) {}

	bool create(context& ctx) {
		return vertex_buffer::create(ctx, sizeof(T));
	}

	bool replace(context& ctx, const T& data) {
		return vertex_buffer::replace(ctx, 0, reinterpret_cast<const uint8_t*>(&data), sizeof(T));
	}

	bool create_or_resize() = delete;
	bool resize() = delete;
};


class device_color_scale {
public:
	static constexpr const char* uniform_name_prefix = "color_scale_arguments";

	void set_uniforms(context& ctx, shader_program& prog, size_t color_scale_index) const {
		bool was_enabled = prog.is_enabled();
		if(!was_enabled)
			prog.enable(ctx);

		auto color_scale = get_color_scale();

		// Todo: Set uniforms that are defined for all color scales.
		const std::string struct_name = std::string(uniform_name_prefix) + "[" + std::to_string(color_scale_index) + "]";
		prog.set_uniform(ctx, struct_name + ".unknown_color", color_scale->get_unknown_color());
		prog.set_uniform(ctx, struct_name + ".domain", color_scale->get_domain());
		prog.set_uniform(ctx, struct_name + ".clamped", color_scale->is_clamped());

		set_color_scale_specific_uniforms(ctx, prog, struct_name);

		if(!was_enabled)
		   prog.disable(ctx);
	}

	//void set_color_scale(std::shared_ptr<cgv::media::color_scale> color_scale) override {
	//}
	

	cgv::data::time_point get_modified_time() const {
		return get_color_scale()->get_modified_time();
	}

	std::vector<cgv::rgba> get_texture_data(size_t texture_resolution) const {
		auto color_scale = get_color_scale();
		if(color_scale)
			return color_scale->quantize(texture_resolution);
		else
			return std::vector<cgv::rgba>(texture_resolution, { 0.0f });
	}

private:
	//virtual std::shared_ptr<const cgv::media::color_scale> get_color_scale() const = 0;
	virtual const cgv::media::color_scale* get_color_scale() const = 0;

	virtual void set_color_scale_specific_uniforms(const context& ctx, shader_program& prog, const std::string& struct_name) const = 0;
};

template<typename T>
class device_color_scale_storage : public device_color_scale {
public:
	device_color_scale_storage() {
		color_scale = std::make_shared<T>();
	}

	//void set_color_scale(std::shared_ptr<cgv::media::color_scale> color_scale) override {
	//	color_scale_ = color_scale;
	//};

	std::shared_ptr<T> color_scale;

private:
	//std::shared_ptr<const cgv::media::color_scale> get_color_scale() const override {
	const cgv::media::color_scale* get_color_scale() const override {
		return color_scale.get();
	};
};

class device_continuous_color_scale : public device_color_scale_storage<cgv::media::continuous_color_scale> {
private:
	void set_color_scale_specific_uniforms(const context& ctx, shader_program& prog, const std::string& struct_name) const override {
		prog.set_uniform(ctx, struct_name + ".transform", static_cast<int>(color_scale->get_transform()));
		prog.set_uniform(ctx, struct_name + ".diverging", color_scale->is_diverging());
		prog.set_uniform(ctx, struct_name + ".midpoint", color_scale->get_midpoint());
		prog.set_uniform(ctx, struct_name + ".exponent", color_scale->get_pow_exponent());
	}
};

class device_transfer_function : public device_color_scale_storage<cgv::media::transfer_function>{
private:
	void set_color_scale_specific_uniforms(const context& ctx, shader_program& prog, const std::string& struct_name) const override {
		prog.set_uniform(ctx, struct_name + ".transform", static_cast<int>(cgv::media::SequentialMappingTransform::kLinear));
		prog.set_uniform(ctx, struct_name + ".diverging", false);
		prog.set_uniform(ctx, struct_name + ".midpoint", 0.5f);
		prog.set_uniform(ctx, struct_name + ".exponent", 1.0f);
	}
};





class color_scale_adapter {
public:
	bool destruct(const context& ctx) {
		return texture_.destruct(ctx);
	}

	bool enable(context& ctx, int texture_unit = -1) {
		create_texture(ctx);
		return texture_.enable(ctx, texture_unit);
	}

	bool disable(const context& ctx) {
		return texture_.disable(ctx);
	}

	void set_uniforms(context& ctx, shader_program& prog, int texture_unit = -1) {
		bool was_enabled = prog.is_enabled();
		if(!was_enabled)
			prog.enable(ctx);
		prog.set_uniform(ctx, "color_scale_texture", texture_unit);

		for(size_t i = 0; i < color_scales_.size(); ++i)
			color_scales_[i]->set_uniforms(ctx, prog, i);

		// Todo: Use uniform buffer object?

		if(!was_enabled)
			prog.disable(ctx);
	}

	void set_color_scale(std::shared_ptr<const device_color_scale> color_scale) {
		if(!color_scales_.size() == 1 || color_scales_.front() != color_scale) {
			auto g = cgv::data::time_point::min();
			build_time_.reset();
			color_scales_ = { color_scale };
		}
	}

	void set_color_scales(const std::vector<std::shared_ptr<const device_color_scale>>& color_scales) {
		if(color_scales_ != color_scales) {
			build_time_.reset();
			color_scales_ = color_scales;
		}
	}

	// Todo: Return const reference. Need to make texture enable/disable const in order to use returned texture.
	// Problem: Not so easy, because enable uses user_data from render_component which cannot be altered if texture_base is const.
	texture& get_texture(const context& ctx) {
		create_texture(ctx);
		return texture_;
	}

private:
	bool create_texture(const context& ctx) {
		if(color_scales_.empty())
			return false;

		cgv::data::time_point latest_time;
		for(const auto& color_scale : color_scales_) {
			if(color_scale->get_modified_time() > latest_time) {
				latest_time = color_scale->get_modified_time();
				break;
			}
		}

		if(!build_time_.is_valid() || latest_time > build_time_.get_modified_time()) {
			const size_t texture_width = 256;

			std::cout << "build color scale resource" << std::endl;

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

	cgv::data::time_stamp build_time_;
	std::vector<std::shared_ptr<const device_color_scale>> color_scales_;
	texture texture_;

};

} // namespace render
} // namespace cgv

#include <cgv/config/lib_end.h>