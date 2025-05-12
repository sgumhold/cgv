#include "compute_kernel.h"

#include <cgv/render/shader_library.h>

namespace cgv {
namespace gpgpu {

bool compute_kernel::init(cgv::render::context& ctx, const std::string& name, const cgv::render::shader_compile_options& config, const std::string& where) {
	return cgv::render::shader_library::load(ctx, _prog, name, config, where);
}

void compute_kernel::destruct(const cgv::render::context& ctx) {
	_prog.destruct(ctx);
}

bool compute_kernel::enable(cgv::render::context& ctx) {
	return _prog.enable(ctx);
}

bool compute_kernel::disable(cgv::render::context& ctx) {
	return _prog.disable(ctx);
}

void compute_kernel::set_arguments(cgv::render::context& ctx, const argument_bindings& arguments) {
	bool was_enabled = false;
	if(_prog.is_enabled())
		was_enabled = true;
	else
		_prog.enable(ctx);

	for(size_t i = 0; i < arguments.get_uniform_count(); ++i) {
		const uniform_binding& binding = arguments.get_uniform(i);
		int loc = _prog.get_uniform_location(ctx, binding._name);
		if(loc > -1)
			_prog.set_uniform(ctx, loc, binding._desc, binding._addr);
	}

	if(!was_enabled)
		_prog.disable(ctx);
}

} // namespace gpgpu
} // namespace cgv
