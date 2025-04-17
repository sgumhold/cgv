#include "algorithm.h"

namespace cgv {
namespace gpgpu {

std::string algorithm::get_type_name() const {
	return _type_name;
}

bool algorithm::is_initialized() const {
	return _is_initialized;
}

void algorithm::destruct(const cgv::render::context& ctx) {
	destruct_kernels(ctx);
}

void algorithm::register_kernel(compute_kernel& kernel, const std::string& name) {
	_kernel_registrations.push_back({ &kernel, name });
};

bool algorithm::init_kernels(cgv::render::context& ctx, const cgv::render::shader_compile_options& config) {
	const std::string debug_context = "cgv::gpgpu::" + get_type_name();
	bool success = true;
	for(const auto& info : _kernel_registrations)
		success &= info.kernel->init(ctx, info.name, config, debug_context);
	_is_initialized = success;
	return success;
}

void algorithm::destruct_kernels(const cgv::render::context& ctx) {
	for(const auto& info : _kernel_registrations)
		info.kernel->destruct(ctx);
	_is_initialized = false;
}

void algorithm::dispatch_compute(unsigned num_groups_x, unsigned num_groups_y, unsigned num_groups_z) {
	glDispatchCompute(num_groups_x, num_groups_y, num_groups_z);
}

} // namespace gpgpu
} // namespace cgv
