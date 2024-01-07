#include "canvas.h"

namespace cgv {
namespace g2d {

canvas::canvas() {
	resolution = ivec2(100);
	origin_setting = Origin::kBottomLeft;
	apply_gamma = false;
	zoom_factor = 1.0f;
	initialize_modelview_matrix_stack();
	current_shader_program = nullptr;
}

void canvas::destruct(cgv::render::context& ctx) {
	shaders.clear(ctx);
}

void canvas::register_shader(const std::string& name, const std::string& filename, bool multi_primitive_mode) {
	cgv::render::shader_define_map defines;
	
	if(!multi_primitive_mode)
		defines["MODE"] = "0";

	shaders.add(name, filename, defines);
}

void canvas::reload_shaders(cgv::render::context& ctx) {
	shaders.reload_all(ctx);
}

bool canvas::init(cgv::render::context& ctx) {
	return shaders.load_all(ctx);
}

bool canvas::has_shader(const std::string& name) const {

	return shaders.contains(name);
}

cgv::render::shader_program& canvas::enable_shader(cgv::render::context& ctx, const std::string& name) {
	auto& prog = shaders.get(name);
	return enable_shader(ctx, prog);
}

cgv::render::shader_program& canvas::enable_shader(cgv::render::context& ctx, cgv::render::shader_program& prog) {
	if(current_shader_program == &prog)
		return prog;

	disable_current_shader(ctx);

	prog.enable(ctx);
	set_view(ctx, prog);
	current_shader_program = &prog;
	return prog;
}

void canvas::disable_current_shader(cgv::render::context& ctx) {
	if(current_shader_program)
		current_shader_program->disable(ctx);
	current_shader_program = nullptr;
}

ivec2 canvas::get_resolution() const {
	return resolution;
}

void canvas::set_resolution(cgv::render::context& ctx, const ivec2& resolution) {
	this->resolution = resolution;
}

Origin canvas::get_origin_setting() const {
	return origin_setting;
}

void canvas::set_origin_setting(Origin origin) {
	origin_setting = origin;
}

void canvas::set_apply_gamma(bool flag) {
	apply_gamma = flag;
}

void canvas::set_zoom_factor(float zoom) {
	zoom_factor = zoom;
}

void canvas::initialize_modelview_matrix_stack() {
	mat3 I = mat3(0.0f);
	I.identity();
	modelview_matrix_stack.push(I);
}

mat3 canvas::get_modelview_matrix() const {
	return modelview_matrix_stack.top();
}

void canvas::set_modelview_matrix(cgv::render::context& ctx, const mat3& M) {
	// set new modelview matrix on matrix stack
	modelview_matrix_stack.top() = M;

	// update in current shader
	if(current_shader_program) {
		if(current_shader_program->is_enabled()) {
			current_shader_program->set_uniform(ctx, "modelview2d_matrix", modelview_matrix_stack.top());
		}
	}
}

void canvas::push_modelview_matrix() {
	modelview_matrix_stack.push(get_modelview_matrix());
}

void canvas::mul_modelview_matrix(cgv::render::context& ctx, const mat3& M) {
	set_modelview_matrix(ctx, get_modelview_matrix()*M);
}

void canvas::pop_modelview_matrix(cgv::render::context& ctx) {
	modelview_matrix_stack.pop();

	if(modelview_matrix_stack.size() == 0)
		initialize_modelview_matrix_stack();

	set_modelview_matrix(ctx, modelview_matrix_stack.top());
}

void canvas::warning(const std::string& what) const {
	std::cerr << "canvas::" + what + " no canvas shader program enabled" << std::endl;
}

void canvas::set_view(cgv::render::context& ctx, cgv::render::shader_program& prog) {
	prog.set_uniform(ctx, "resolution", resolution);
	prog.set_uniform(ctx, "modelview2d_matrix", modelview_matrix_stack.top());
	prog.set_uniform(ctx, "origin_upper_left", origin_setting == Origin::kTopLeft);
	prog.set_uniform(ctx, "apply_gamma", apply_gamma);
	prog.set_uniform(ctx, "zoom_factor", zoom_factor);
}

void canvas::set_style(cgv::render::context& ctx, const shape2d_style& style) {
	if(current_shader_program)
		style.apply(ctx, *current_shader_program);
}

}
}
