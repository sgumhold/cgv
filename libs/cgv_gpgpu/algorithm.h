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

/// The base class for compute shader based highly parallel GPU algorithms.
class CGV_API algorithm {
public:
	algorithm(const std::string& type_name) : _type_name(type_name) {}
	
	std::string get_type_name() const;

	bool is_initialized() const;

protected:
	struct compute_kernel_info {
		compute_kernel* kernel = nullptr;
		std::string name;
		cgv::render::shader_define_map defines;
	};

	bool init(cgv::render::context& ctx, const std::vector<compute_kernel_info>& kernel_infos, const cgv::render::shader_compile_options& config);

	void destruct(const cgv::render::context& ctx);

	void set_buffer_binding_indices(const sl::named_buffer_list& buffers, uint32_t base_index);
	
	void set_image_binding_indices(const sl::named_image_list& images, uint32_t base_index);
	
	void set_texture_binding_indices(const sl::named_texture_list& textures, uint32_t base_index);

	struct algorithm_configuration_info {
		const argument_definitions* arguments = nullptr;
		std::vector<sl::data_type> types;
		uint32_t default_buffer_count = 0;
		uint32_t default_image_count = 0;
		uint32_t default_texture_count = 0;
	};

	cgv::render::shader_compile_options configure(const algorithm_configuration_info& info);

	cgv::render::shader_compile_options get_configuration(const argument_definitions& arguments, const std::vector<sl::data_type> types = {}) const;

	bool is_valid_range(device_buffer_iterator first, device_buffer_iterator last);

	void bind_buffer_like_arguments(cgv::render::context& ctx, const argument_bindings& arguments);

	void unbind_buffer_like_arguments(cgv::render::context& ctx, const argument_bindings& arguments);

	void bind_buffer_arguments(cgv::render::context& ctx, const argument_bindings& arguments);
	
	void unbind_buffer_arguments(cgv::render::context& ctx, const argument_bindings& arguments);

	//void bind_image_arguments(cgv::render::context& ctx, const argument_bindings& arguments);
	//
	//void unbind_image_arguments(cgv::render::context& ctx, const argument_bindings& arguments);

	void dispatch_compute(unsigned num_groups_x, unsigned num_groups_y, unsigned num_groups_z);

private:
	
	const std::string _type_name;
	bool _is_initialized = false;
	std::map<std::string, uint32_t> _buffer_binding_indices;
	uint32_t _base_buffer_binding_index = 0;
	std::map<std::string, uint32_t> _image_binding_indices;
	uint32_t _base_image_binding_index = 0;
	std::map<std::string, uint32_t> _texture_binding_indices;
	uint32_t _base_texture_binding_index = 0;
};

} // namespace gpgpu
} // namespace cgv

#include <cgv/config/lib_end.h>
