#include "msdf_gl_font_renderer.h"

namespace cgv {
namespace g2d {

bool msdf_gl_font_renderer::build_shader_program(const cgv::render::context& ctx) {
	return prog.build_program(ctx, shader_prog_name, true);
}

vec2 msdf_gl_font_renderer::alignment_to_offset(cgv::render::TextAlignment alignment) {
	vec2 offset(-0.5f);

	if(alignment & cgv::render::TA_LEFT)
		offset.x() = 0.0f;
	else if(alignment & cgv::render::TA_RIGHT)
		offset.x() = -1.0f;

	if(alignment & cgv::render::TA_TOP)
		offset.y() = -1.0f;
	else if(alignment & cgv::render::TA_BOTTOM)
		offset.y() = 0.0f;

	return offset;
}

void msdf_gl_font_renderer::draw_text(cgv::render::context& ctx, const msdf_font& font, const vec3& position, const msdf_text_geometry::text_info& text_info, const text_render_info& render_info) {
	vec2 text_size(text_info.normalized_width, 1.0f);
	text_size *= render_info.scale;

	vec2 percentual_offset = alignment_to_offset(render_info.alignment);
	percentual_offset.y() *= font.get_cap_height();

	prog.set_uniform(ctx, "text_size", text_size);
	prog.set_uniform(ctx, "translation", position);
	prog.set_uniform(ctx, "rotation", render_info.rotation);
	prog.set_uniform(ctx, "color", render_info.color);
	prog.set_uniform(ctx, "percentual_offset", percentual_offset);

	glDrawArraysInstancedBaseInstance(GL_TRIANGLE_STRIP, (GLint)0, (GLsizei)4, (GLsizei)text_info.str.length(), (GLuint)text_info.offset);
}

void msdf_gl_font_renderer::manage_singleton(cgv::render::context& ctx, const std::string& name, int& ref_count, int ref_count_change) {
	switch(ref_count_change) {
	case 1:
		if(ref_count == 0) {
			if(!init(ctx))
				ctx.error("unable to initialize " + name + " singleton");
		}
		++ref_count;
		break;
	case 0:
		break;
	case -1:
		if(ref_count == 0)
			ctx.error("attempt to decrease reference count of " + name + " singleton below 0");
		else {
			if(--ref_count == 0)
				destruct(ctx);
		}
		break;
	default:
		ctx.error("invalid change reference count outside {-1,0,1} for " + name + " singleton");
	}
}

bool msdf_gl_font_renderer::init(cgv::render::context& ctx) {
	attribute_arrays.init(ctx);
	return build_shader_program(ctx);
}

void msdf_gl_font_renderer::destruct(cgv::render::context& ctx) {
	attribute_arrays.destruct(ctx);
	prog.destruct(ctx);
}

cgv::render::shader_program& msdf_gl_font_renderer::ref_prog() {
	return prog;
}

bool msdf_gl_font_renderer::enable(cgv::render::context& ctx, msdf_font& font, const text2d_style& style) {
	bool res = prog.is_enabled() ? true : prog.enable(ctx);
	res &= font.enable(ctx);
	if(res) {
		prog.set_uniform(ctx, "src_size", font.get_initial_font_size());
		prog.set_uniform(ctx, "pixel_range", font.get_pixel_range());
		style.apply(ctx, prog);
	}
	return res;
}

bool msdf_gl_font_renderer::enable(cgv::render::context& ctx, msdf_text_geometry& tg, const text2d_style& style) {
	return tg.enable(ctx) && enable(ctx, tg.ref_msdf_font(), style);
}

bool msdf_gl_font_renderer::disable(cgv::render::context& ctx, msdf_font& font) {
	font.disable(ctx);
	return prog.disable(ctx);
}

bool msdf_gl_font_renderer::disable(cgv::render::context& ctx, msdf_text_geometry& tg) {
	tg.disable(ctx);
	return disable(ctx, tg.ref_msdf_font());
}

void msdf_gl_font_renderer::draw(cgv::render::context& ctx, msdf_font& font, const std::string& text, const vec3& position, const text_render_info& render_info) {
	if(text.empty())
		return;

	std::vector<cgv::vec4> quads;
	std::vector<cgv::vec4> texcoords;
	font.generate_vertex_data(text, quads, texcoords);
	attribute_arrays.set_attribute_array(ctx, 0, quads);
	attribute_arrays.set_attribute_array(ctx, 1, texcoords);
	
	if(attribute_arrays.enable(ctx)) {
		// advance vertex attributes once per instance
		glVertexAttribDivisor(0, 1);
		glVertexAttribDivisor(1, 1);

		msdf_text_geometry::text_info text_info;
		text_info.str = text;
		text_info.offset = 0;
		text_info.normalized_width = font.compute_normalized_length(text);
		draw_text(ctx, font, position, text_info, render_info);

		attribute_arrays.disable(ctx);
	}
}

void msdf_gl_font_renderer::draw(cgv::render::context& ctx, msdf_text_geometry& tg, size_t offset, int count) {
	if(offset >= tg.size())
		return;

	size_t end = count < 0 ? tg.size() : offset + static_cast<size_t>(count);
	end = std::min(end, tg.size());

	const auto& font = tg.ref_msdf_font();

	for(size_t i = offset; i < end; ++i) {
		text_render_info render_info;
		render_info.scale = tg.get_scale(i);
		render_info.rotation = tg.get_rotation(i);
		render_info.color = tg.get_color(i);
		render_info.alignment = tg.get_alignment(i);
		
		draw_text(ctx, font, tg.get_position(i), tg.get_text_info(i), render_info);
	}
}

bool msdf_gl_font_renderer::render(cgv::render::context& ctx, msdf_font& font, const std::string& text, const vec3& position, const text_render_info& render_info, const text2d_style& style) {
	if(enable(ctx, font, style)) {
		draw(ctx, font, text, position, render_info);
		return disable(ctx, font);
	}
	return false;
}

bool msdf_gl_font_renderer::render(cgv::render::context& ctx, msdf_text_geometry& tg, const text2d_style& style, size_t offset, int count) {
	if(enable(ctx, tg, style)) {
		draw(ctx, tg, offset, count);
		return disable(ctx, tg);
	}
	return false;
}



msdf_gl_font_renderer_2d::msdf_gl_font_renderer_2d() : msdf_gl_font_renderer() {
	shader_prog_name = "sdf_font2d.glpr";
}

bool msdf_gl_font_renderer_2d::enable(cgv::render::context& ctx, canvas& cvs, msdf_font& font, const text2d_style& style) {
	bool res = msdf_gl_font_renderer::enable(ctx, font, style);
	if(res)
		cvs.set_view(ctx, prog);
	return res;
}

bool msdf_gl_font_renderer_2d::enable(cgv::render::context& ctx, canvas& cvs, msdf_text_geometry& tg, const text2d_style& style) {
	bool res = msdf_gl_font_renderer::enable(ctx, tg, style);
	if(res)
		cvs.set_view(ctx, prog);
	return res;
}

void msdf_gl_font_renderer_2d::draw(cgv::render::context& ctx, canvas& cvs, msdf_font& font, const std::string& text, const vec2& position, const text_render_info& render_info) {
	cvs.set_view(ctx, prog);
	msdf_gl_font_renderer::draw(ctx, font, text, vec3(position, 0.0), render_info);
}

void msdf_gl_font_renderer_2d::draw(cgv::render::context& ctx, canvas& cvs, msdf_text_geometry& tg, size_t offset, int count) {
	cvs.set_view(ctx, prog);
	msdf_gl_font_renderer::draw(ctx, tg, offset, count);
}

bool msdf_gl_font_renderer_2d::render(cgv::render::context& ctx, canvas& cvs, msdf_font& font, const std::string& text, const vec2& position, const text_render_info& render_info, const text2d_style& style) {
	if(!enable(ctx, cvs, font, style))
		return false;
	msdf_gl_font_renderer::draw(ctx, font, text, vec3(position, 0.0f), render_info);
	return disable(ctx, font);
}

bool msdf_gl_font_renderer_2d::render(cgv::render::context& ctx, canvas& cvs, msdf_text_geometry& tg, const text2d_style& style, size_t offset, int count) {
	if(!enable(ctx, cvs, tg, style))
		return false;
	msdf_gl_font_renderer::draw(ctx, tg, offset, count);
	return disable(ctx, tg);
}

msdf_gl_font_renderer_2d& ref_msdf_gl_font_renderer_2d(cgv::render::context& ctx, int ref_count_change) {
	static int ref_count = 0;
	static msdf_gl_font_renderer_2d r;
	r.manage_singleton(ctx, "msdf_gl_font_renderer_2d", ref_count, ref_count_change);
	return r;
}



msdf_gl_font_renderer_3d::msdf_gl_font_renderer_3d() : msdf_gl_font_renderer() {
	shader_prog_name = "sdf_font3d.glpr";
}

msdf_gl_font_renderer_3d& ref_msdf_gl_font_renderer_3d(cgv::render::context& ctx, int ref_count_change) {
	static int ref_count = 0;
	static msdf_gl_font_renderer_3d r;
	r.manage_singleton(ctx, "msdf_gl_font_renderer_3d", ref_count, ref_count_change);
	return r;
}

}
}
