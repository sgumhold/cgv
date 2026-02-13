#pragma once

#include <vector>

#include <cgv/data/time_stamp.h>

#include "device_color_scale.h"
#include "shader_program.h"
#include "vertex_buffer.h"

#include "lib_begin.h"

namespace cgv {
namespace render {


// Todo: Which name prefix to choose? gl, device, gpu?

// Todo: Move uniform buffer implementation from gpgpu to render and remove this temporary local copy. Make context const

template<class T>
class uniform_buffer : public vertex_buffer {
public:
	uniform_buffer(VertexBufferUsage usage = VertexBufferUsage::VBU_STREAM_COPY) : vertex_buffer(VertexBufferType::VBT_UNIFORM, usage) {}

	bool create(const context& ctx) {
		return vertex_buffer::create(ctx, sizeof(T));
	}

	bool create(const context& ctx, size_t array_size) {
		return vertex_buffer::create(ctx, sizeof(T) * array_size);
	}

	bool resize(const context& ctx, size_t array_size) {
		return vertex_buffer::resize(ctx, sizeof(T) * array_size);
	}

	bool resize(const context& ctx, const T& data) {
		return vertex_buffer::resize(ctx, reinterpret_cast<const uint8_t*>(&data), sizeof(T));
	}

	bool resize(const context& ctx, const std::vector<T>& array) {
		return vertex_buffer::resize(ctx, reinterpret_cast<const uint8_t*>(array.data()), sizeof(T) * array.size());
	}

	bool replace(const context& ctx, const T& data) {
		return vertex_buffer::replace(ctx, 0, reinterpret_cast<const uint8_t*>(&data), sizeof(T));
	}

	bool replace(const context& ctx, const std::vector<T>& array) {
		return vertex_buffer::replace(ctx, 0, reinterpret_cast<const uint8_t*>(array.data()), sizeof(T) * array.size());
	}

	bool resize_or_replace(const context& ctx, const T& data) {
		return size_in_bytes != sizeof(T) ? resize(ctx, data) : replace(ctx, data);
	}

	bool resize_or_replace(const context& ctx, std::vector<T>& array) {
		return size_in_bytes != sizeof(T) ? resize(ctx, array) : replace(ctx, array);
	}

	bool create_or_resize() = delete;
};

class CGV_API color_scale_adapter {
public:
	bool init(const context& ctx);

	bool destruct(const context& ctx);

	bool enable(const context& ctx, int texture_unit);

	bool disable(const context& ctx);

	void set_uniforms_in_program(context& ctx, shader_program& prog, int texture_unit);

	void set_color_scale(std::shared_ptr<const device_color_scale> color_scale);

	void set_color_scales(const std::vector<std::shared_ptr<const device_color_scale>>& color_scales);

	// Todo: Return const reference. Need to make texture enable/disable const in order to use returned texture.
	// Problem: Not so easy, because enable uses user_data from render_component which cannot be altered if texture_base is const.
	texture& get_texture(const context& ctx);

private:
	cgv::data::time_point get_latest_color_scale_modification_time() const;

	bool create_texture(const context& ctx);

	cgv::data::time_stamp build_time_;
	std::vector<std::shared_ptr<const device_color_scale>> color_scales_;
	texture texture_ = { "uint8[R,G,B,A]" };
	std::vector<device_color_scale_arguments> uniforms_;
	uniform_buffer<device_color_scale_arguments> uniform_buffer_;
};

} // namespace render
} // namespace cgv

#include <cgv/config/lib_end.h>
