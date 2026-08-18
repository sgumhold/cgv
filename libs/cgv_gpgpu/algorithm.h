#pragma once

#include <limits>
#include <system_error>

#include <cgv/render/context.h>
#include <cgv_gl/gl/gl.h>

#include "compute_kernel.h"
#include "device_buffer_iterator.h"
#include "error.h"
#include "representation.h"

#include "lib_begin.h"

#define CGV_GPGPU_DISABLE_DERIVED_TYPES(TYPE) typename std::enable_if<!std::is_base_of<TYPE, T>::value, bool>::type = true

namespace cgv {
namespace gpgpu {

enum class GroupSize : uint32_t {
	Auto = 0,		// Automatically determine group size based on algorithm type and/or device capabilities; currently only used in texture_algorithm (the automatic tuning of group sizes for the used device is not yet implemented)
	k32 = 32,
	k64 = 64,
	k128 = 128,
	k256 = 256,
	k512 = 512,
	k1024 = 1024
};

extern CGV_API GLbitfield get_associated_memory_barrier_bits(cgv::render::VertexBufferType buffer_type);

constexpr static GroupSize k_default_group_size = GroupSize::k256;

/// The base class for compute shader based highly parallel GPU algorithms.
class CGV_API algorithm {
public:
	algorithm(const std::string& type_name, GroupSize group_size) : _type_name(type_name), _group_size(static_cast<uint32_t>(group_size)) {}
	
	std::string get_type_name() const;

	bool is_initialized() const;

	error get_last_error() const {
		return _last_error;
	}

protected:
	uint32_t _group_size = static_cast<uint32_t>(k_default_group_size);

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
		std::string name {};
		cgv::render::shader_compile_options options {};
	};

	cgv::render::shader_compile_options get_compile_options(const algorithm_create_info& create_info);

	bool init(cgv::render::context& ctx, const algorithm_create_info& create_info, const std::vector<compute_kernel_info>& kernel_infos);

	void destruct(const cgv::render::context& ctx);

	/// Return true if iterators first and last point to the same range and distance(first, last) > 0.
	bool is_valid_range(device_buffer_iterator first, device_buffer_iterator last) const;

	void bind_buffer_like_arguments(cgv::render::context& ctx, const argument_bindings& arguments);

	void unbind_buffer_like_arguments(cgv::render::context& ctx, const argument_bindings& arguments);

	void dispatch_compute(unsigned num_groups_x, unsigned num_groups_y, unsigned num_groups_z);

	static void optional_memory_barrier(cgv::render::VertexBufferType buffer_type);

	void raise_error(errc e, const std::string& description = "");

	void raise_unsupported_type_error(const sl::data_type& type);

	void raise_not_enough_shared_memory_error(size_t available, size_t required);

private:
	std::string get_type_error_description(const sl::data_type& type) const;

	error _last_error;

	const std::string _type_name;
	bool _is_initialized = false;
	std::map<std::string, uint32_t> _buffer_binding_indices;
	std::map<std::string, uint32_t> _image_binding_indices;
	std::map<std::string, uint32_t> _texture_binding_indices;
};

} // namespace gpgpu
} // namespace cgv

#include <cgv/config/lib_end.h>
