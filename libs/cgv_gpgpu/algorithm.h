#pragma once

#include <cgv/render/context.h>
#include <cgv_gl/gl/gl.h>

#include "sl.h"
#include "utils.h"
#include "compute_kernel.h"

#include "lib_begin.h"

namespace cgv {
namespace gpgpu {

/** Definition of base functionality for highly parallel gpu algorithms. */
class CGV_API algorithm {
public:
	// TODO: Remove default constructor.
	algorithm() {}
	algorithm(const std::string& type_name) : _type_name(type_name) {}
	
	// TODO: Move to private access after refactoring all algorithms.
	bool _is_initialized = false;

	std::string get_type_name() const;

	bool is_initialized() const;

	void destruct(const cgv::render::context& ctx);

protected:
	void register_kernel(compute_kernel& kernel, const std::string& name);
	
	bool init_kernels(cgv::render::context& ctx, const cgv::render::shader_compile_options& config);

	void destruct_kernels(const cgv::render::context& ctx);

	void dispatch_compute(unsigned num_groups_x, unsigned num_groups_y, unsigned num_groups_z);

private:
	struct compute_kernel_info {
		compute_kernel* kernel = nullptr;
		std::string name;
	};

	std::string _type_name;
	
	std::vector<compute_kernel_info> _kernel_registrations;
};

} // namespace gpgpu
} // namespace cgv

#include <cgv/config/lib_end.h>
