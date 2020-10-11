#include "rectangle_renderer.h"

namespace trajectory {
	rectangle_renderer::rectangle_renderer() : rect(null_rectangle) {}

	rectangle_renderer::rectangle_renderer(const rectangle &_rect) : rect(_rect) {}

	rectangle_renderer::rectangle_renderer(std::shared_ptr<texture> tex)
		: rect(null_rectangle), tex(tex)
	{
	}

	rectangle_renderer::rectangle_renderer(const rectangle &rect,
		std::shared_ptr<texture> tex)
		: rect(rect), tex(tex)
	{
	}

	rectangle_renderer::~rectangle_renderer() {}

	bool rectangle_renderer::init(context &ctx)
	{

		bool success = vao.create(ctx);
		success = prog.build_program(ctx, "rectangle_blit.glpr", true) && success;

		prog.set_uniform(ctx, "draw_mode", 0);
		prog.set_uniform(ctx, "tex", 0);
		prog.set_uniform(ctx, "zoom", zoom);
		prog.set_uniform(ctx, "offset", offset);

		std::vector<vec3> v = { rect[0], rect[1], rect[2], rect[3] };
		v_buf.create(ctx, v.data(), v.size());

		if (!success) {
			std::cerr << "couldn't initialize rectangle blitter" << std::endl;
		}

		return success;
	}

	void rectangle_renderer::init_frame(context &ctx) {}

	void rectangle_renderer::draw(context &ctx)
	{
		if (!tex) return;

		draw_impl(ctx, rect, *tex);
	}

	void rectangle_renderer::draw(context &ctx, const rectangle &rect, texture &tex)
	{
		draw_impl(ctx, rect, tex);
	}

	void rectangle_renderer::draw_fullscreen(context &ctx)
	{
		if (!tex) return;

		draw_fullscreen_impl(ctx, *tex);
	}

	void rectangle_renderer::draw_fullscreen(context &ctx, texture &tex)
	{
		draw_fullscreen_impl(ctx, tex);
	}

	void rectangle_renderer::draw_impl(context &ctx, const rectangle &rect,
		texture &tex)
	{
		vao.enable(ctx);

		std::vector<vec3> v = { rect[0], rect[1], rect[2], rect[3] };
		v_buf.replace(ctx, 0, v.data(), v.size());
		type_descriptor v_type(cgv::type::info::TI_FLT32, 3u, false);
		vao.set_attribute_array(ctx, 0, v_type, v_buf, 0u, 0u, 0u);

		prog.set_uniform(ctx, "fullscreen", false);
		if (mode_changed) {
			set_draw_mode_uniforms(ctx);
			mode_changed = false;
		}

		prog.enable(ctx);
		tex.enable(ctx, 0);

		// attributeless rendering
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		tex.disable(ctx);
		prog.disable(ctx);

		vao.disable(ctx);
	}

	void rectangle_renderer::draw_fullscreen_impl(context &ctx, texture &tex)
	{
		glDisable(GL_CULL_FACE);
		vao.enable(ctx);

		std::vector<vec3> v = { vec3(-1.0, 1.0, 0.0), vec3(1.0, 1.0, 0.0),
							   vec3(-1.0, -1.0, 0.0), vec3(1.0, -1.0, 0.0) };
		v_buf.replace(ctx, 0, v.data(), v.size());
		type_descriptor v_type(cgv::type::info::TI_FLT32, 3u, false);
		vao.set_attribute_array(ctx, 0, v_type, v_buf, 0u, 0u, 0u);

		prog.set_uniform(ctx, "fullscreen", true);
		if (mode_changed) {
			set_draw_mode_uniforms(ctx);
			mode_changed = false;
		}

		prog.enable(ctx);
		tex.enable(ctx, 0);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		tex.disable(ctx);
		prog.disable(ctx);

		vao.disable(ctx);
		glEnable(GL_CULL_FACE);
	}

void rectangle_renderer::draw(context &ctx, const rectangle &rect,
                              GLuint texture_handle)
{
	vao.enable(ctx);

	std::vector<vec3> v = {rect[0], rect[1], rect[2], rect[3]};
	v_buf.replace(ctx, 0, v.data(), v.size());
	type_descriptor v_type(cgv::type::info::TI_FLT32, 3u, false);
	vao.set_attribute_array(ctx, 0, v_type, v_buf, 0u, 0u, 0u);

	prog.set_uniform(ctx, "fullscreen", false);
	if (mode_changed) {
		set_draw_mode_uniforms(ctx);
		mode_changed = false;
	}

	prog.enable(ctx);
	glBindTexture(GL_TEXTURE_2D, texture_handle);
	glActiveTexture(GL_TEXTURE0);

	// attributeless rendering
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glBindTexture(GL_TEXTURE_2D, 0);
	prog.disable(ctx);

	vao.disable(ctx);
}

void rectangle_renderer::draw_fullscreen(context &ctx, GLuint texture_handle)
{
	glDisable(GL_CULL_FACE);
	vao.enable(ctx);

	std::vector<vec3> v = {vec3(-1.0, 1.0, 0.0), vec3(1.0, 1.0, 0.0),
	                       vec3(-1.0, -1.0, 0.0), vec3(1.0, -1.0, 0.0)};
	v_buf.replace(ctx, 0, v.data(), v.size());
	type_descriptor v_type(cgv::type::info::TI_FLT32, 3u, false);
	vao.set_attribute_array(ctx, 0, v_type, v_buf, 0u, 0u, 0u);

	prog.set_uniform(ctx, "fullscreen", true);
	if (mode_changed) {
		set_draw_mode_uniforms(ctx);
		mode_changed = false;
	}

	prog.enable(ctx);
	glBindTexture(GL_TEXTURE_2D, texture_handle);
	glActiveTexture(GL_TEXTURE0);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glBindTexture(GL_TEXTURE_2D, 0);
	prog.disable(ctx);

	vao.disable(ctx);

	glEnable(GL_CULL_FACE);
}

void rectangle_renderer::clear(context &ctx)
{
	// do not delete texture

	prog.destruct(ctx);

	v_buf.destruct(ctx);
	vao.destruct(ctx);
}

void rectangle_renderer::set_draw_mode(draw_mode m)
{
	mode = m;
	mode_changed = true;
}

void rectangle_renderer::set_flipped(bool f)
{
	flipped = f;
	mode_changed = true;
}

rectangle_renderer::vec2 rectangle_renderer::get_zoom() const { return zoom; }

void rectangle_renderer::set_zoom(const vec2 zoom)
{
	this->zoom = zoom;
	mode_changed = true;
}

rectangle_renderer::vec2 rectangle_renderer::get_offset() const { return offset; }

void rectangle_renderer::set_offset(const vec2 offset)
{
	this->offset = offset;
	mode_changed = true;
}

void rectangle_renderer::set_draw_mode_uniforms(context &ctx)
{
	if (!prog.is_created()) return;

	prog.set_uniform(ctx, "flipped", flipped);
	prog.set_uniform(ctx, "zoom", zoom);
	prog.set_uniform(ctx, "offset", offset);

	switch (mode) {
	case draw_mode::NORMAL: {
		prog.set_uniform(ctx, "draw_mode", 0);
		break;
	}
	case draw_mode::UPPER_HALF: {
		prog.set_uniform(ctx, "draw_mode", 1);
		break;
	}
	case draw_mode::LOWER_HALF: {
		prog.set_uniform(ctx, "draw_mode", 2);
		break;
	}
	case draw_mode::LEFT_HALF: {
		prog.set_uniform(ctx, "draw_mode", 3);
		break;
	}
	case draw_mode::RIGHT_HALF: {
		prog.set_uniform(ctx, "draw_mode", 4);
		break;
	}
	}
}


std::shared_ptr<texture> rectangle_renderer::get_texture() const { return tex; }

void rectangle_renderer::set_texture(std::shared_ptr<texture> t) { tex = t; }

rectangle rectangle_renderer::get_rectangle() const { return rect; }

void rectangle_renderer::set_rectangle(const rectangle &_rect) { rect = _rect; }


} // namespace trajectory
