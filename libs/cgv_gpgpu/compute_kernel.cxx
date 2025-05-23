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
	for(size_t i = 0; i < arguments.get_uniform_count(); ++i) {
		const uniform_binding* binding = arguments.get_uniform(i);
		int loc = _prog.get_uniform_location(ctx, binding->name());
		if(loc > -1)
			_prog.set_uniform(ctx, loc, binding->descriptor(), binding->address());
	}
}

} // namespace gpgpu
} // namespace cgv
