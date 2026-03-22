#pragma once

#include <vector>

#include <cgv/data/time_stamp.h>

#include "device_color_scale.h"
#include "shader_program.h"
#include "uniform_buffer.h"

#include "lib_begin.h"

namespace cgv {
namespace render {

/// @brief Manages graphics resources to enable using device_color_scale in shader programs.
///
/// color_scale_adapter can store a reference to one or more device_color_scales and handles automatic texture
/// and uniform buffer creation to enable using color_scales in shader programs that use the interface
/// from color_scale.glsl. The texture is only updated if new device_color_scales are set or the referenced
/// device_color_scales are modified as detected through color_scale::get_modified_time().
class CGV_API color_scale_adapter {
public:
	/// @brief Initialize the color_scale_adapter.
	/// Initializes the used buffers. Needs to be called first before any other method.
	/// 
	/// @param ctx The CGV rendering context.
	/// @return True if successful, false otherwise.
	bool init(const context& ctx);

	/// @brief Destruct the color_scale_adapter.
	/// Destructs the used buffers. No method may be called after this, except init().
	/// 
	/// @param ctx The CGV rendering context.
	/// @return True if successful, false otherwise.
	bool destruct(const context& ctx);

	/// @brief Enable the managed render resources to prepare for rendering.
	/// 
	/// @param ctx The CGV rendering context.
	/// @param texture_unit The texture unit used for the color scale texture. Must match the value used in set_uniforms_in_program().
	/// @return True if successful, false otherwise.
	bool enable(const context& ctx, int texture_unit);

	/// @brief Disable the managed render resources after rendering.
	/// 
	/// @param ctx The CGV rendering context.
	/// @return True if successful, false otherwise.
	bool disable(const context& ctx);

	/// @brief Set texture unit and uniform buffer location in the given shader_program and prepare uniform buffer.
	/// 
	/// @param ctx The CGV rendering context.
	/// @param prog The shader_program used for rendering. Must be linked against color_scale.glsl.
	/// @param texture_unit The texture unit used for the color scale texture. Must match the value used in enable().
	void set_uniforms_in_program(context& ctx, shader_program& prog, int texture_unit);

	/// @brief Set a reference to a single device_color_scale to be managed by this color_scale_adapter.
	/// 
	/// @param color_scale The adapted device_color_scale.
	void set_color_scale(std::shared_ptr<const device_color_scale> color_scale);

	/// @brief Set a reference to multiple device_color_scales to be managed by this color_scale_adapter.
	/// Will only store references to the first get_supported_color_scale_count() passed device_color_scales.
	/// 
	/// @param color_scales The adapted device_color_scales.
	void set_color_scales(const std::vector<std::shared_ptr<const device_color_scale>>& color_scales);

	/// @brief Get the number of maximally supported device_color_scales.
	/// 
	/// @return The count.
	size_t get_supported_color_scale_count() const {
		return k_max_color_scale_count_;
	}

	/// @brief Get the texture containing the device_color_scales' color data.
	/// 
	/// @param ctx The CGV rendering context.
	/// @return The texture.
	/// Todo: Return as const reference. Need to make texture enable/disable const to be able to use the returned texture.
	/// However, this is not so simple as texture::enable() calls a method from the context which will modify texture_base
	/// which is then also const.
	texture& get_texture(const context& ctx);

private:
	/// The maximum number of simultaneously supported device_color_scales as governed by color_scale.glsl.
	static const size_t k_max_color_scale_count_ = 4;
	/// The width of the texture and thus the maximum sample count of color scale data.
	static const size_t k_texture_width_ = 256;

	/// @brief Get the latest modification time among all managed device_color_scales.
	/// 
	/// @return The time_point.
	cgv::data::time_point get_latest_color_scale_modification_time() const;

	/// @brief Create the texture containing the device_color_scales' color data.
	/// Each device_color_scale's color data will occupy one row of the texture.
	/// The height of the texture is equivalent to the number of managed device_color_scales.
	/// 
	/// @param ctx The CGV rendering context.
	/// @return True if successful, false otherwise.
	bool create_texture(const context& ctx);

	/// The build time of the texture. Used to test againt the managed device_color_scales modification times.
	cgv::data::time_stamp build_time_;
	/// The managed device_color_scales.
	std::vector<std::shared_ptr<const device_color_scale>> color_scales_;
	/// The texture containing the device_color_scales' color data.
	texture texture_ = { "uint8[R,G,B,A]" };
	/// An array of device_color_scale_arguments holding the mapping arguments.
	std::vector<device_color_scale_arguments> uniforms_;
	/// The uniform buffer used to upload device_color_scale_arguments to the graphics device.
	uniform_buffer<device_color_scale_arguments> uniform_buffer_;
};

} // namespace render
} // namespace cgv

#include <cgv/config/lib_end.h>
