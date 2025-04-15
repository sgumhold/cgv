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

void compute_kernel::set_argument_locations(cgv::render::context& ctx, const std::string& prefix, uniform_arguments& arguments) const {
	for(uniform_binding* binding : arguments._bindings)
		binding->_loc = _prog.get_uniform_location(ctx, prefix + binding->_name);
}

void compute_kernel::set_arguments(cgv::render::context& ctx, const uniform_arguments& arguments) {
	enable_guard guard(ctx, _prog);
	for(const uniform_binding* binding : arguments._bindings)
		_prog.set_uniform(ctx, binding->_loc, binding->_desc, binding->_addr);
}

void compute_kernel::set_arguments(cgv::render::context& ctx, const uniform_binding_list& arguments, const std::string& prefix) {
	enable_guard guard(ctx, _prog);
	for(const uniform_binding& binding : arguments)
		_prog.set_uniform(ctx, _prog.get_uniform_location(ctx, prefix + binding._name), binding._desc, binding._addr);
}

compute_kernel::enable_guard::enable_guard(cgv::render::context& ctx, cgv::render::shader_program& prog) : ctx(ctx), prog(prog) {
	if(prog.is_enabled())
		was_enabled = true;
	else
		prog.enable(ctx);
}

compute_kernel::enable_guard::~enable_guard() {
	if(!was_enabled)
		prog.disable(ctx);
}

}
}
