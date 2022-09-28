#pragma once

#include <cgv/render/context.h>
#include <cgv/render/render_types.h>
#include <cgv/render/shader_program.h>
#include <cgv_gl/gl/gl_context.h>
#include <cgv_glutil/2d/rect.h>

#include "../shader_library.h"

#include "../lib_begin.h"

namespace cgv {
namespace glutil {

class CGV_API canvas : public cgv::render::render_types {
public:
	struct CGV_API shaders_2d {
		static const std::string arrow;
		static const std::string background;
		static const std::string circle;
		static const std::string cubic_spline;
		static const std::string ellipse;
		static const std::string grid;
		static const std::string line;
		static const std::string polygon;
		static const std::string quad;
		static const std::string rectangle;
	};

protected:
	cgv::glutil::shader_library shaders;
	ivec2 resolution;
	float feather_scale;
	bool apply_gamma;

	std::stack<mat3> modelview_matrix_stack;

	cgv::render::shader_program* current_shader_program;

public:
	canvas() {
		resolution = ivec2(100);
		feather_scale = 1.0f;
		apply_gamma = true;
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

	void reload_shaders(cgv::render::context& ctx) {
		shaders.reload_all(ctx);
	}

	bool init(cgv::render::context& ctx) {
		return shaders.load_all(ctx);
	}

	cgv::render::shader_program& enable_shader(cgv::render::context& ctx, const std::string& name) {
		disable_current_shader(ctx);

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

	ivec2 get_resolution() const {
		return resolution;
	}

	void set_resolution(cgv::render::context& ctx, const ivec2& resolution) {
		this->resolution = resolution;
	}

	void set_feather_scale(float s) {
		feather_scale = s;
	}

	void set_apply_gamma(bool flag) {
		apply_gamma = flag;
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

	void warning(const std::string& what) {
		std::cerr << "canvas::" + what + " no canvas shader program enabled" << std::endl;
	}

	void set_view(cgv::render::context& ctx, cgv::render::shader_program& prog) {
		prog.set_uniform(ctx, "resolution", resolution);
		prog.set_uniform(ctx, "modelview2d_matrix", modelview_matrix_stack.top());
		prog.set_uniform(ctx, "feather_scale", feather_scale);
		prog.set_uniform(ctx, "apply_gamma", apply_gamma);
	}

	void draw_shape(const cgv::render::context& ctx, const rect& r) {
		draw_shape(ctx, r.pos(), r.size());
	}

	template<typename T>
	void draw_shape(const cgv::render::context& ctx, const cgv::math::fvec<T, 2u>& position, const cgv::math::fvec<T, 2u>& size) {
		if(current_shader_program) {
			current_shader_program->set_attribute(ctx, "position", static_cast<cgv::math::fvec<float, 2u>>(position));
			current_shader_program->set_attribute(ctx, "size", static_cast<cgv::math::fvec<float, 2u>>(size));
			glDrawArrays(GL_POINTS, 0, 1);
		} else {
			warning("draw_shape");
		}
	}

	template<typename T>
	void draw_shape(const cgv::render::context& ctx, const cgv::math::fvec<T, 2u>& position, const cgv::math::fvec<T, 2u>& size, const rgba& color) {
		if(current_shader_program) {
			current_shader_program->set_attribute(ctx, "position", static_cast<cgv::math::fvec<float, 2u>>(position));
			current_shader_program->set_attribute(ctx, "size", static_cast<cgv::math::fvec<float, 2u>>(size));
			current_shader_program->set_attribute(ctx, "color", color);
			glDrawArrays(GL_POINTS, 0, 1);
		} else {
			warning("draw_shape");
		}
	}

	void draw_shape(const cgv::render::context& ctx, const rect& r, const rgba& color) {
		draw_shape(ctx, r.pos(), r.size(), color);
	}

	template<typename T>
	void draw_shape2(const cgv::render::context& ctx, const cgv::math::fvec<T, 2u>& position0, const cgv::math::fvec<T, 2u>& position1) {
		if(current_shader_program) {
			current_shader_program->set_attribute(ctx, "position0", static_cast<cgv::math::fvec<float, 2u>>(position0));
			current_shader_program->set_attribute(ctx, "position1", static_cast<cgv::math::fvec<float, 2u>>(position1));
			glDrawArrays(GL_POINTS, 0, 1);
		} else {
			warning("draw_shape2");
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
		} else {
			warning("draw_shape2");
		}
	}

	template<typename T>
	void draw_shape4(const cgv::render::context& ctx, const cgv::math::fvec<T, 2u>& position0, const cgv::math::fvec<T, 2u>& position1, const cgv::math::fvec<T, 2u>& position2, const cgv::math::fvec<T, 2u>& position3) {
		if(current_shader_program) {
			current_shader_program->set_attribute(ctx, "position0", static_cast<cgv::math::fvec<float, 2u>>(position0));
			current_shader_program->set_attribute(ctx, "position1", static_cast<cgv::math::fvec<float, 2u>>(position1));
			current_shader_program->set_attribute(ctx, "position2", static_cast<cgv::math::fvec<float, 2u>>(position2));
			current_shader_program->set_attribute(ctx, "position3", static_cast<cgv::math::fvec<float, 2u>>(position3));
			glDrawArrays(GL_POINTS, 0, 1);
		} else {
			warning("draw_shape4");
		}
	}

	template<typename T>
	void draw_shape4(const cgv::render::context& ctx, const cgv::math::fvec<T, 2u>& position0, const cgv::math::fvec<T, 2u>& position1, const cgv::math::fvec<T, 2u>& position2, const cgv::math::fvec<T, 2u>& position3, const rgba& color) {
		if(current_shader_program) {
			current_shader_program->set_attribute(ctx, "position0", static_cast<cgv::math::fvec<float, 2u>>(position0));
			current_shader_program->set_attribute(ctx, "position1", static_cast<cgv::math::fvec<float, 2u>>(position1));
			current_shader_program->set_attribute(ctx, "position2", static_cast<cgv::math::fvec<float, 2u>>(position2));
			current_shader_program->set_attribute(ctx, "position3", static_cast<cgv::math::fvec<float, 2u>>(position3));
			current_shader_program->set_attribute(ctx, "color", color);
			glDrawArrays(GL_POINTS, 0, 1);
		} else {
			warning("draw_shape4");
		}
	}
};

}
}

#include <cgv/config/lib_end.h>
