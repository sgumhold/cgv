#include "msdf_gl_font_renderer.h"

namespace cgv {
namespace glutil {

msdf_gl_font_renderer& ref_msdf_gl_font_renderer(cgv::render::context& ctx, int ref_count_change) {
	static int ref_count = 0;
	static msdf_gl_font_renderer r;
	r.manage_singleton(ctx, "msdf_gl_font_renderer", ref_count, ref_count_change);
	return r;
}

bool msdf_gl_font_renderer::build_shader_program(const cgv::render::context& ctx) {
	return prog.build_program(ctx, "sdf_font2d.glpr", true);
}

void msdf_gl_font_renderer::manage_singleton(cgv::render::context& ctx, const std::string& name, int& ref_count, int ref_count_change) {
	switch(ref_count_change) {
	case 1:
		if(ref_count == 0) {
			if(!init(ctx))
				ctx.error(std::string("unable to initialize ") + name + " singleton");
		}
		++ref_count;
		break;
	case 0:
		break;
	case -1:
		if(ref_count == 0)
			ctx.error(std::string("attempt to decrease reference count of ") + name + " singleton below 0");
		else {
			if(--ref_count == 0)
				destruct(ctx);
		}
		break;
	default:
		ctx.error(std::string("invalid change reference count outside {-1,0,1} for ") + name + " singleton");
	}
}

void msdf_gl_font_renderer::destruct(cgv::render::context& ctx) {
	prog.destruct(ctx);
}

bool msdf_gl_font_renderer::init(cgv::render::context& ctx) {
	return build_shader_program(ctx);
}

cgv::render::shader_program& msdf_gl_font_renderer::ref_prog() {
	return prog;
}

//cgv::render::shader_program& msdf_gl_font_renderer::enable_prog(cgv::render::context& ctx) {
//	prog.enable(ctx);
//	return prog;
//}

bool msdf_gl_font_renderer::enable(cgv::render::context& ctx, const ivec2& viewport_resolution, msdf_text_geometry& tg, const shape2d_style& style) {
	bool res = prog.is_enabled() ? true : prog.enable(ctx);
	res &= tg.enable(ctx);
	if(res) {
		prog.set_uniform(ctx, "resolution", viewport_resolution);
		prog.set_uniform(ctx, "src_size", tg.get_msdf_font()->get_initial_font_size());
		prog.set_uniform(ctx, "pixel_range", tg.get_msdf_font()->get_pixel_range());
		prog.set_uniform(ctx, "true_sdf_mix_factor", 0.0f);
		style.apply(ctx, prog);
	}
	return res;
}

bool msdf_gl_font_renderer::disable(cgv::render::context& ctx, msdf_text_geometry& tg) {
	bool res = prog.disable(ctx);
	tg.disable(ctx);
	return res;
}

void msdf_gl_font_renderer::draw(cgv::render::context& ctx, msdf_text_geometry& tg, size_t offset, int count) {
	if(offset >= tg.size())
		return;

	size_t end = count < 0 ? tg.size() : offset + static_cast<size_t>(count);
	end = std::min(end, tg.size());

	for(size_t i = offset; i < end; ++i) {
		const auto& text = tg.ref_texts()[i];

		ivec4 position_offset;
		position_offset.x() = round(text.position.x());
		position_offset.y() = round(text.position.y());

		vec2 half_size = 0.5f * text.size * tg.get_font_size();
		vec2 offset = -half_size;

		if(text.alignment & cgv::render::TA_LEFT)
			offset.x() += half_size.x();
		else if(text.alignment & cgv::render::TA_RIGHT)
			offset.x() -= half_size.x();

		if(text.alignment & cgv::render::TA_TOP)
			offset.y() -= half_size.y();
		else if(text.alignment & cgv::render::TA_BOTTOM)
			offset.y() += half_size.y();

		position_offset.z() = round(offset.x());
		position_offset.w() = round(offset.y());

		prog.set_uniform(ctx, "position_offset", position_offset);
		prog.set_uniform(ctx, "font_size", tg.get_font_size());
		prog.set_uniform(ctx, "angle", text.angle);

		glDrawArraysInstancedBaseInstance(GL_TRIANGLE_STRIP, (GLint)0, (GLsizei)4, (GLsizei)text.str.size(), (GLuint)text.offset);
	}
}

bool msdf_gl_font_renderer::render(cgv::render::context& ctx, const ivec2& viewport_resolution, msdf_text_geometry& tg, const shape2d_style& style, size_t offset, int count) {
	if(!enable(ctx, viewport_resolution, tg, style))
		return false;
	draw(ctx, tg, offset, count);
	return disable(ctx, tg);
}

}
}
