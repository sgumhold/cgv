#pragma once

#include <limits>

#include <cgv/render/context.h>
#include <cgv_gl/gl/gl.h>

#include "device_buffer_iterator.h"
#include "compute_kernel.h"
#include "sl.h"

#include "lib_begin.h"

#define CGV_GPGPU_DISABLE_DERIVED_TYPES(TYPE) typename std::enable_if<!std::is_base_of<TYPE, T>::value, bool>::type = true

namespace cgv {
namespace gpgpu {

extern CGV_API GLbitfield get_associated_memory_barrier_bits(cgv::render::VertexBufferType buffer_type);

constexpr static uint32_t k_default_group_size = 256;

/// The base class for compute shader based highly parallel GPU algorithms.
class CGV_API algorithm {
public:
	// TODO: remove default value for group_size
	algorithm(const std::string& type_name, uint32_t group_size = k_default_group_size) : _type_name(type_name), _group_size(group_size) {}
	
	std::string get_type_name() const;

	bool is_initialized() const;

protected:
	uint32_t _group_size = k_default_group_size;

	struct algorithm_create_info {
		const argument_definitions* arguments = nullptr;
		std::vector<sl::data_type> types;
		std::vector<std::pair<std::string, sl::data_type>> typedefs;
		uint32_t default_buffer_count = 0;
		uint32_t default_image_count = 0;
		uint32_t default_texture_count = 0;
		cgv::render::shader_compile_options options;
	};

	struct compute_kernel_info {
		compute_kernel* kernel = nullptr;
		std::string name;
		cgv::render::shader_compile_options options;
	};

	cgv::render::shader_compile_options get_compile_options(const algorithm_create_info& create_info);

	bool init(cgv::render::context& ctx, const algorithm_create_info& create_info, const std::vector<compute_kernel_info>& kernel_infos);

	void destruct(const cgv::render::context& ctx);

	bool is_valid_range(device_buffer_iterator first, device_buffer_iterator last);

	void bind_buffer_like_arguments(cgv::render::context& ctx, const argument_bindings& arguments);

	void unbind_buffer_like_arguments(cgv::render::context& ctx, const argument_bindings& arguments);

	void dispatch_compute(unsigned num_groups_x, unsigned num_groups_y, unsigned num_groups_z);

	static void optional_memory_barrier(cgv::render::VertexBufferType buffer_type);

private:
	const std::string _type_name;
	bool _is_initialized = false;
	std::map<std::string, uint32_t> _buffer_binding_indices;
	std::map<std::string, uint32_t> _image_binding_indices;
	std::map<std::string, uint32_t> _texture_binding_indices;
};

} // namespace gpgpu
} // namespace cgv

#include <cgv/config/lib_end.h>
