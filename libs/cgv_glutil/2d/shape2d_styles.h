#pragma once

#include <cgv/render/context.h>
#include <cgv/render/render_types.h>
#include <cgv/render/shader_program.h>
#include <cgv_gl/gl/gl_context.h>

#include "../shader_library.h"

//#include "../lib_begin.h"

namespace cgv {
namespace glutil {

class canvas : public cgv::render::render_types {
protected:
	cgv::glutil::shader_library shaders;
	ivec2 resolution;
	float feather_scale;

	std::stack<mat3> modelview_matrix_stack;

	cgv::render::shader_program* current_shader_program;

public:
	canvas() {
		resolution = ivec2(100);
		feather_scale = 1.0f;
		initialize_modelview_matrix_stack();
		current_shader_program = nullptr;
	}

	~canvas() {}

	void destruct(cgv::render::context& ctx) {
		shaders.clear(ctx);
	}

	void register_shader(const std::string& name, const std::string& filename, bool multi_primitive_mode = false) {
		cgv::render::shader_define_map defines;
		if(!multi_primitive_mode) {
			defines["MODE"] = "0";
		}
		shaders.add(name, filename, defines);
	}

	bool init(cgv::render::context& ctx) {
		return shaders.load_shaders(ctx);
	}

	cgv::render::shader_program& enable_shader(cgv::render::context& ctx, const std::string& name) {
		auto& prog = shaders.get(name);
		prog.enable(ctx);
		set_view(ctx, prog);
		current_shader_program = &prog;
		return prog;
	}

	void disable_current_shader(cgv::render::context& ctx) {
		if(current_shader_program)
			current_shader_program->disable(ctx);
		current_shader_program = nullptr;
	}

	void set_resolution(cgv::render::context& ctx, const ivec2& resolution) {
		this->resolution = resolution;

		for(auto it = shaders.begin(); it != shaders.end(); ++it) {
			auto& prog = it->second.prog;
			prog.enable(ctx);
			prog.set_uniform(ctx, "resolution", resolution);
			prog.disable(ctx);
		}
	}

	void set_feather_scale(float s) {
		feather_scale = s;
	}

	void initialize_modelview_matrix_stack() {
		mat3 I = mat3(0.0f);
		I.identity();
		modelview_matrix_stack.push(I);
	}

	mat3 get_modelview_matrix() {
		return modelview_matrix_stack.top();
	}

	void set_modelview_matrix(cgv::render::context& ctx, const mat3& M) {
		// set new modelview matrix on matrix stack
		modelview_matrix_stack.top() = M;

		// update in current shader
		if(current_shader_program) {
			if(current_shader_program->is_enabled()) {
				current_shader_program->set_uniform(ctx, "modelview2d_matrix", modelview_matrix_stack.top());
			}
		}
	}

	void push_modelview_matrix() {
		modelview_matrix_stack.push(get_modelview_matrix());
	}

	void mul_modelview_matrix(cgv::render::context& ctx, const mat3& M) {
		set_modelview_matrix(ctx, get_modelview_matrix()*M);
	}

	void pop_modelview_matrix(cgv::render::context& ctx) {
		modelview_matrix_stack.pop();
		
		if(modelview_matrix_stack.size() == 0)
			initialize_modelview_matrix_stack();

		set_modelview_matrix(ctx, modelview_matrix_stack.top());
	}

	void set_view(cgv::render::context& ctx, cgv::render::shader_program& prog) {
		prog.set_uniform(ctx, "modelview2d_matrix", modelview_matrix_stack.top());
		prog.set_uniform(ctx, "feather_scale", feather_scale);
	}

	template<typename T>
	void draw_shape(const cgv::render::context& ctx, const cgv::math::fvec<T, 2u>& position, const cgv::math::fvec<T, 2u>& size) {
		if(current_shader_program) {
			current_shader_program->set_attribute(ctx, "position", static_cast<cgv::math::fvec<float, 2u>>(position));
			current_shader_program->set_attribute(ctx, "size", static_cast<cgv::math::fvec<float, 2u>>(size));
			glDrawArrays(GL_POINTS, 0, 1);
		}
	}

	template<typename T>
	void draw_shape(const cgv::render::context& ctx, const cgv::math::fvec<T, 2u>& position, const cgv::math::fvec<T, 2u>& size, const rgba& color) {
		if(current_shader_program) {
			current_shader_program->set_attribute(ctx, "position", static_cast<cgv::math::fvec<float, 2u>>(position));
			current_shader_program->set_attribute(ctx, "size", static_cast<cgv::math::fvec<float, 2u>>(size));
			current_shader_program->set_attribute(ctx, "color", color);
			glDrawArrays(GL_POINTS, 0, 1);
		}
	}

	template<typename T>
	void draw_shape2(const cgv::render::context& ctx, const cgv::math::fvec<T, 2u>& position0, const cgv::math::fvec<T, 2u>& position1) {
		if(current_shader_program) {
			current_shader_program->set_attribute(ctx, "position0", static_cast<cgv::math::fvec<float, 2u>>(position0));
			current_shader_program->set_attribute(ctx, "position1", static_cast<cgv::math::fvec<float, 2u>>(position1));
			glDrawArrays(GL_POINTS, 0, 1);
		}
	}

	template<typename T>
	void draw_shape2(const cgv::render::context& ctx, const cgv::math::fvec<T, 2u>& position0, const cgv::math::fvec<T, 2u>& position1, const rgba& color0, const rgba& color1) {
		if(current_shader_program) {
			current_shader_program->set_attribute(ctx, "position0", static_cast<cgv::math::fvec<float, 2u>>(position0));
			current_shader_program->set_attribute(ctx, "position1", static_cast<cgv::math::fvec<float, 2u>>(position1));
			current_shader_program->set_attribute(ctx, "color0", color0);
			current_shader_program->set_attribute(ctx, "color1", color1);
			glDrawArrays(GL_POINTS, 0, 1);
		}
	}
};

struct shape2d_style : cgv::render::render_types {
	// placement options
	bool position_is_center = false;
	// appearance options
	rgba fill_color = rgba(1.0f);
	rgba border_color = rgba(0.0f, 0.0f, 0.0f, 1.0f);
	float border_width = 0.0f;
	float border_radius = 0.0f;
	float ring_width = 0.0f;
	float feather_width = 1.0f;
	float feather_origin = 0.5f;
	vec2 texcoord_scaling = vec2(1.0f);
	// render options
	bool use_fill_color = true;
	bool use_texture = false;
	bool use_blending = false;
	bool use_smooth_feather = false;
	bool apply_gamma = true; // TOOD: maybe move to global 2d uniforms

	void apply(cgv::render::context & ctx, cgv::render::shader_program& prog) {
		prog.set_uniform(ctx, "position_is_center", position_is_center);

		prog.set_uniform(ctx, "fill_color", fill_color);
		prog.set_uniform(ctx, "border_color", border_color);
		prog.set_uniform(ctx, "border_width", border_width);
		prog.set_uniform(ctx, "border_radius", border_radius);
		prog.set_uniform(ctx, "ring_width", ring_width);
		prog.set_uniform(ctx, "feather_width", feather_width);
		prog.set_uniform(ctx, "feather_origin", feather_origin);
		prog.set_uniform(ctx, "tex_scaling", texcoord_scaling);

		prog.set_uniform(ctx, "use_fill_color", use_fill_color);
		prog.set_uniform(ctx, "use_texture", use_texture);
		prog.set_uniform(ctx, "use_blending", use_blending);
		prog.set_uniform(ctx, "use_smooth_feather", use_smooth_feather);
		prog.set_uniform(ctx, "apply_gamma", apply_gamma);
	}
};

struct line2d_style : public shape2d_style {
	float width = 1.0f;
	float dash_length = 0.0f;
	float dash_ratio = 0.5f;

	void apply(cgv::render::context & ctx, cgv::render::shader_program& prog) {
		shape2d_style::apply(ctx, prog);

		prog.set_uniform(ctx, "width", width);
		prog.set_uniform(ctx, "dash_length", dash_length);
		prog.set_uniform(ctx, "dash_ratio", dash_ratio);
	}
};

struct arrow2d_style : public shape2d_style {
	float stem_width = 1.0f;
	float head_width = 2.0f;
	float absolute_head_length = 0.5f;
	float relative_head_length = 0.5f;
	bool head_length_is_relative = true;

	void apply(cgv::render::context & ctx, cgv::render::shader_program& prog) {
		shape2d_style::apply(ctx, prog);

		prog.set_uniform(ctx, "stem_width", stem_width);
		prog.set_uniform(ctx, "head_width", head_width);
		prog.set_uniform(ctx, "head_length", head_length_is_relative ? relative_head_length : absolute_head_length);
		prog.set_uniform(ctx, "head_length_is_relative", head_length_is_relative);
	}
};

}
}

#include <cgv/config/lib_end.h>
