#pragma once

#include <cgv/render/context.h>
#include <cgv/render/shader_library.h>
#include <cgv_gl/gl/gl_context.h>
#include <cgv_g2d/trect.h>
#include <cgv_g2d/shaders2d.h>
#include <cgv_g2d/shape2d_styles.h>
#include <cgv_g2d/utils2d.h>

#include "lib_begin.h"

namespace cgv {
namespace g2d {

class CGV_API canvas {
protected:
	cgv::render::shader_library shaders;

	ivec2 resolution;
	Origin origin_setting;
	bool apply_gamma;
	float zoom_factor;

	std::stack<mat3> modelview_matrix_stack;

	cgv::render::shader_program* current_shader_program;

public:
	canvas();

	~canvas() {}

	void destruct(cgv::render::context& ctx);

	void register_shader(const std::string& name, const std::string& filename, bool multi_primitive_mode = false);

	void reload_shaders(cgv::render::context& ctx);

	bool init(cgv::render::context& ctx);

	bool has_shader(const std::string& name) const;

	cgv::render::shader_program& enable_shader(cgv::render::context& ctx, const std::string& name);

	cgv::render::shader_program& enable_shader(cgv::render::context& ctx, cgv::render::shader_program& prog);

	void disable_current_shader(cgv::render::context& ctx);

	ivec2 get_resolution() const;

	void set_resolution(cgv::render::context& ctx, const ivec2& resolution);

	Origin get_origin_setting() const;

	void set_origin_setting(Origin origin);

	void set_apply_gamma(bool flag);

	void set_zoom_factor(float zoom);

	void initialize_modelview_matrix_stack();

	mat3 get_modelview_matrix() const;

	void set_modelview_matrix(cgv::render::context& ctx, const mat3& M);

	void push_modelview_matrix();

	void mul_modelview_matrix(cgv::render::context& ctx, const mat3& M);

	void pop_modelview_matrix(cgv::render::context& ctx);

	void warning(const std::string& what) const;

	void set_view(cgv::render::context& ctx, cgv::render::shader_program& prog);

	void set_style(cgv::render::context& ctx, const shape2d_style& style);

	template<typename T>
	void draw_shape(const cgv::render::context& ctx, const trect<T>& r) {
		draw_shape(ctx, r.position, r.size);
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

	template<typename T>
	void draw_shape(const cgv::render::context& ctx, const trect<T>& r, const rgba& color) {
		draw_shape(ctx, r.position, r.size, color);
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
