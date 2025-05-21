#pragma once

#include <cgv/render/context.h>
#include <cgv_gl/gl/gl.h>

#include "sl.h"
#include "compute_kernel.h"

#include "lib_begin.h"

namespace cgv {
namespace gpgpu {

/// The base class for compute shader based highly parallel GPU algorithms.
class CGV_API algorithm {
public:
	algorithm(const std::string& type_name) : _type_name(type_name) {}
	
	std::string get_type_name() const;

	bool is_initialized() const;

	void destruct(const cgv::render::context& ctx);

protected:
	void register_kernel(compute_kernel& kernel, const std::string& name);
	void register_kernel(compute_kernel& kernel, const std::string& name, const cgv::render::shader_define_map& defines);
	
	bool init_kernels(cgv::render::context& ctx, const cgv::render::shader_compile_options& config);

	void destruct_kernels(const cgv::render::context& ctx);

	void set_buffer_binding_indices(const sl::named_buffer_list& buffers, uint32_t base_index);

	cgv::render::shader_compile_options get_configuration(const argument_definitions& arguments, const std::vector<sl::data_type> types = {}) const;

	void bind_buffer_arguments(cgv::render::context& ctx, const argument_bindings& arguments);
	
	void unbind_buffer_arguments(cgv::render::context& ctx, const argument_bindings& arguments);

	void dispatch_compute(unsigned num_groups_x, unsigned num_groups_y, unsigned num_groups_z);

private:
	struct compute_kernel_info {
		compute_kernel* kernel = nullptr;
		std::string name;
		cgv::render::shader_define_map defines;
	};

	const std::string _type_name;
	bool _is_initialized = false;
	std::vector<compute_kernel_info> _kernel_registrations;
	std::map<std::string, uint32_t> _buffer_binding_indices;
	uint32_t _base_buffer_binding_index = 0;
};

} // namespace gpgpu
} // namespace cgv

#include <cgv/config/lib_end.h>
