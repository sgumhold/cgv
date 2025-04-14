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

void compute_kernel::set_arguments(cgv::render::context& ctx, const uniform_argument_list& arguments) {
	bool was_enabled = _prog.is_enabled();
	if(!was_enabled)
		_prog.enable(ctx);

	for(const uniform_argument& arg : arguments)
		_prog.set_uniform(ctx, _prog.get_uniform_location(ctx, arg.name), arg.desc, arg.addr);

	if(!was_enabled)
		_prog.disable(ctx);
}

}
}
