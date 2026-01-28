#pragma once

#include <vector>

#include <cgv/data/time_stamp.h>
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

	void set_color_scale(std::shared_ptr<const cgv::media::color_scale> color_scale) {
		if(!color_scales_.size() == 1 || color_scales_.front() != color_scale) {
			auto g = cgv::data::time_point::min();
			build_time.reset();
			color_scales_ = { color_scale };
		}
	}

	void set_color_scales(const std::vector<std::shared_ptr<const cgv::media::color_scale>>& color_scales) {
		if(color_scales_ != color_scales) {
			build_time.reset();
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

		if(!build_time.is_valid() || latest_time > build_time.get_modified_time()) {
			const size_t max_resolution = 256;

			std::cout << "build color scale resource" << std::endl;

			std::vector<cgv::rgba8> texture_data;

			for(const auto& color_scale : color_scales_) {
				size_t offset = texture_data.size();
				texture_data.resize(texture_data.size() + max_resolution);

				std::vector<rgba> colors = color_scale->quantize(max_resolution);
				std::transform(colors.begin(), colors.end(), texture_data.begin() + offset, [](const cgv::rgba& color) {
					return cgv::rgba8(color);
				});
			}

			cgv::data::data_format data_format(max_resolution, color_scales_.size(), cgv::type::info::TI_UINT8, cgv::data::CF_RGBA);
			cgv::data::data_view data_view(&data_format, texture_data.data());

			// Todo: Use nearest filter for scales with discrete color ramps.
			const cgv::render::TextureFilter filter = cgv::render::TF_LINEAR;
			texture_.set_min_filter(filter);
			texture_.set_mag_filter(filter);

			build_time.modified();
			return texture_.create(ctx, data_view, 0);
		}




		/*
		bool out_of_date = false;
		if(color_scales_.size() != created_color_scales_.size()) {
			out_of_date = true;
		} else {
			for(size_t i = 0; i < color_scales_.size(); ++i) {
				if(color_scales_[i] != created_color_scales_[i]) {
					out_of_date = true;
					break;
				}
				if(color_scales_[i]->get_modified_time() > created_times_[i]) {
					out_of_date = true;
					break;
				}
			}
		}

		if(out_of_date) {
			std::cout << "build color scale resource" << std::endl;

			out_of_date = false;

			created_color_scales_ = color_scales_;
			created_times_.clear();
			for(size_t i = 0; i < color_scales_.size(); ++i)
				created_times_.push_back(color_scales_[i]->get_modified_time());

			std::vector<cgv::rgba8> texture_data;

			for(const auto& color_scale : color_scales_) {
				size_t offset = texture_data.size();
				texture_data.resize(texture_data.size() + max_resolution);

				std::vector<rgba> colors = color_scale->quantize(max_resolution);
				std::transform(colors.begin(), colors.end(), texture_data.begin() + offset, [](const cgv::rgba& color) {
					return cgv::rgba8(color);
				});
			}

			cgv::data::data_view data_view(new cgv::data::data_format(max_resolution, color_scales_.size(), cgv::type::info::TI_UINT8, cgv::data::CF_RGBA), texture_data.data());

			// Todo: Use nearest filter for scales with discrete color ramps.
			const cgv::render::TextureFilter filter = cgv::render::TF_LINEAR;
			texture_.set_min_filter(filter);
			texture_.set_mag_filter(filter);
			return texture_.create(ctx, data_view, 0);


		}
		*/

		return true;



		/*
		// Todo: remove print after debug
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
		*/
	}

	std::vector<std::shared_ptr<const cgv::media::color_scale>> color_scales_;
	texture texture_;

	//std::vector<std::shared_ptr<const cgv::media::color_scale>> created_color_scales_;
	//std::vector<cgv::media::time_stamp::time_point_type> created_times_;
	cgv::data::time_stamp build_time;

};

//extern CGV_API void configure_color_scales(cgv::render::context& ctx, cgv::render::shader_program& prog, const std::vector<cgv::media::color_scale*>& color_scales);

} // namespace render
} // namespace cgv

#include <cgv/config/lib_end.h>