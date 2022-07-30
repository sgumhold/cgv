#pragma once

#include <cgv/render/context.h>
#include <cgv/render/render_types.h>
#include <cgv/render/shader_program.h>
#include <cgv_gl/gl/gl_context.h>

#include "../shader_library.h"

namespace cgv {
namespace glutil {

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
	vec2 texcoord_offset = vec2(0.0f);
	// render options
	bool use_fill_color = true;
	bool use_texture = false;
	bool use_texture_alpha = true;
	bool use_blending = false;
	bool use_smooth_feather = false;
	bool apply_gamma = true; // TOOD: maybe move to global 2d uniforms

	virtual void apply(cgv::render::context & ctx, cgv::render::shader_program& prog) {
		prog.set_uniform(ctx, "position_is_center", position_is_center);

		prog.set_uniform(ctx, "fill_color", fill_color);
		prog.set_uniform(ctx, "border_color", border_color);
		prog.set_uniform(ctx, "border_width", border_width);
		prog.set_uniform(ctx, "border_radius", border_radius);
		prog.set_uniform(ctx, "ring_width", ring_width);
		prog.set_uniform(ctx, "feather_width", feather_width);
		prog.set_uniform(ctx, "feather_origin", feather_origin);
		prog.set_uniform(ctx, "tex_scaling", texcoord_scaling);
		prog.set_uniform(ctx, "tex_offset", texcoord_offset);

		prog.set_uniform(ctx, "use_fill_color", use_fill_color);
		prog.set_uniform(ctx, "use_texture", use_texture);
		prog.set_uniform(ctx, "use_texture_alpha", use_texture_alpha);
		prog.set_uniform(ctx, "use_blending", use_blending);
		prog.set_uniform(ctx, "use_smooth_feather", use_smooth_feather);
		prog.set_uniform(ctx, "apply_gamma", apply_gamma);
	}
};

struct line2d_style : public shape2d_style {
	float width = 1.0f;
	float dash_length = 0.0f;
	float dash_ratio = 0.5f;

	virtual void apply(cgv::render::context & ctx, cgv::render::shader_program& prog) {
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

	virtual void apply(cgv::render::context & ctx, cgv::render::shader_program& prog) {
		shape2d_style::apply(ctx, prog);

		prog.set_uniform(ctx, "stem_width", stem_width);
		prog.set_uniform(ctx, "head_width", head_width);
		prog.set_uniform(ctx, "head_length", head_length_is_relative ? relative_head_length : absolute_head_length);
		prog.set_uniform(ctx, "head_length_is_relative", head_length_is_relative);
	}
};

struct grid2d_style : public shape2d_style {
	enum GridPattern {
		GP_GRID,
		GP_SQUARES,
		GP_CHECKER
	};
	GridPattern pattern = GP_GRID;
	float scale = 0.5f;

	virtual void apply(cgv::render::context & ctx, cgv::render::shader_program& prog) {
		shape2d_style::apply(ctx, prog);

		prog.set_uniform(ctx, "pattern", static_cast<int>(pattern));
		prog.set_uniform(ctx, "scale", scale);
	}
};

}
}
