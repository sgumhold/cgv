#include "compute_kernel.h"

#include <cgv/render/shader_library.h>

namespace cgv {
namespace gpgpu {

bool compute_kernel::init(cgv::render::context& ctx, const std::string& name, const cgv::render::shader_compile_options& config, const std::string& where) {
	bool res = cgv::render::shader_library::load(ctx, _prog, name, config, where);
	if(res)
		_uniforms = get_program_uniforms(ctx, _prog);
	return res;
}

void compute_kernel::destruct(const cgv::render::context& ctx) {
	_prog.destruct(ctx);
	_uniforms.clear();
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

	for(const uniform_argument& arg : arguments) {
		auto it = _uniforms.find(arg.name);
		if(it != uniforms.end()) {
			int loc = it->second;
			_prog.set_uniform(ctx, loc, arg.desc, arg.addr);
		}
	}

	if(!was_enabled)
		_prog.disable(ctx);
}

}
}
