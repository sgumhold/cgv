#pragma once

#include <memory>

#include <cgv/render/attribute_array_binding.h>
#include <cgv/render/shader_program.h>
#include <cgv/render/vertex_buffer.h>
#include <cgv/render/drawable.h>
#include <cgv_gl/gl/gl.h>

#include "rectangle.h"
#include "placeable.h"

namespace trajectory {


class rectangle_renderer : public cgv::render::drawable
{
  public:
	enum draw_mode : uint8_t {
		NORMAL = 0,
		UPPER_HALF,
		LOWER_HALF,
		LEFT_HALF,
		RIGHT_HALF
	};

  private:
	cgv::data::rectangle rect;
	draw_mode mode = draw_mode::NORMAL;
	bool mode_changed = false;
	bool flipped = false;

	cgv::vec2 zoom = { 0.0f };
	cgv::vec2 offset = { 0.0f };

	// does not own texture
	std::shared_ptr<cgv::render::texture> tex;

	cgv::render::attribute_array_binding vao;
	cgv::render::vertex_buffer v_buf{ cgv::render::VertexBufferType::VBT_VERTICES,
						cgv::render::VertexBufferUsage::VBU_DYNAMIC_DRAW};

	cgv::render::shader_program prog;

  public:
	rectangle_renderer();
	rectangle_renderer(const cgv::data::rectangle &rect);
	rectangle_renderer(std::shared_ptr<cgv::render::texture> tex);
	rectangle_renderer(const cgv::data::rectangle &rect, std::shared_ptr<cgv::render::texture> tex);
	~rectangle_renderer();

	bool init(cgv::render::context &ctx);
	void init_frame(cgv::render::context &ctx);

	// draw rectangle at position/orientation given by placeable
	// draw with member texture
	void draw(cgv::render::context &ctx);
	// draw with given texture
	void draw(cgv::render::context &ctx, const cgv::data::rectangle &rectangle, cgv::render::texture &texture);
	// draw with given handle
	void draw(cgv::render::context &ctx, const cgv::data::rectangle &rectangle, GLuint texture_handle);

	// draw fullscreen, ignores placeable attributes
	// draw with member texture
	void draw_fullscreen(cgv::render::context &ctx);
	// draw with given texture
	void draw_fullscreen(cgv::render::context &ctx, cgv::render::texture &texture);
	// draw with given handle
	void draw_fullscreen(cgv::render::context &ctx, GLuint texture_handle);

	void clear(cgv::render::context &ctx);

	// mainly used for left/right eye
	void set_draw_mode(draw_mode mode);

	void set_flipped(bool flipped);

	std::shared_ptr<cgv::render::texture> get_texture() const;
	void set_texture(std::shared_ptr<cgv::render::texture> t);

	cgv::data::rectangle get_rectangle() const;
	void set_rectangle(const cgv::data::rectangle &rectangle);

	cgv::vec2 get_zoom() const;
	void set_zoom(const cgv::vec2 zoom);

	cgv::vec2 get_offset() const;
	void set_offset(const cgv::vec2 offset);

  private:
	void draw_impl(cgv::render::context &ctx, const cgv::data::rectangle &rectangle, cgv::render::texture &texture);
	void draw_fullscreen_impl(cgv::render::context &ctx, cgv::render::texture &texture);

	void set_draw_mode_uniforms(cgv::render::context &ctx);

};


} // namespace trajectory
