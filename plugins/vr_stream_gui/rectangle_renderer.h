#pragma once

#include <memory>

#include <cgv/render/attribute_array_binding.h>
#include <cgv/render/shader_program.h>
#include <cgv/render/vertex_buffer.h>
#include <cgv/render/drawable.h>
#include <cgv_gl/gl/gl.h>

#include "rectangle.h"
#include "placeable.h"

using namespace cgv::data;
using namespace cgv::math;
using namespace cgv::render;

namespace trajectory {


class rectangle_renderer : public drawable
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
	rectangle rect;
	draw_mode mode = draw_mode::NORMAL;
	bool mode_changed = false;
	bool flipped = false;

	vec2 zoom = vec2(0.0f, 0.0f);
	vec2 offset = vec2(0.0f, 0.0f);

	// does not own texture
	std::shared_ptr<texture> tex;

	attribute_array_binding vao;
	vertex_buffer v_buf{VertexBufferType::VBT_VERTICES,
	                    VertexBufferUsage::VBU_DYNAMIC_DRAW};

	shader_program prog;

  public:
	rectangle_renderer();
	rectangle_renderer(const rectangle &rect);
	rectangle_renderer(std::shared_ptr<texture> tex);
	rectangle_renderer(const rectangle &rect, std::shared_ptr<texture> tex);
	~rectangle_renderer();

	bool init(context &ctx);
	void init_frame(context &ctx);

	// draw rectangle at position/orientation given by placeable
	// draw with member texture
	void draw(context &ctx);
	// draw with given texture
	void draw(context &ctx, const rectangle &rectangle, texture &texture);
	// draw with given handle
	void draw(context &ctx, const rectangle &rectangle, GLuint texture_handle);

	// draw fullscreen, ignores placeable attributes
	// draw with member texture
	void draw_fullscreen(context &ctx);
	// draw with given texture
	void draw_fullscreen(context &ctx, texture &texture);
	// draw with given handle
	void draw_fullscreen(context &ctx, GLuint texture_handle);

	void clear(context &ctx);

	// mainly used for left/right eye
	void set_draw_mode(draw_mode mode);

	void set_flipped(bool flipped);

	std::shared_ptr<texture> get_texture() const;
	void set_texture(std::shared_ptr<texture> t);

	rectangle get_rectangle() const;
	void set_rectangle(const rectangle &rectangle);

	vec2 get_zoom() const;
	void set_zoom(const vec2 zoom);

	vec2 get_offset() const;
	void set_offset(const vec2 offset);

  private:
	void draw_impl(context &ctx, const rectangle &rectangle, texture &texture);
	void draw_fullscreen_impl(context &ctx, texture &texture);

	void set_draw_mode_uniforms(context &ctx);

};


} // namespace trajectory
