#include "cgv_gl/gl/gl.h"

#include "virtual_display.h"
#include "debug_draw.h"

namespace vr {
namespace room {
	virtual_display::virtual_display() : display(plane{})
	{
		blitter = std::make_shared<trajectory::rectangle_renderer>();
		set_position(cgv::vec3(0, 1, 0));
		set_orientation(cgv::quat(0.0f, 0.0f, -1.0f, 0.0f));
	}
	virtual_display::virtual_display(const plane &display) : display(display)
	{
		blitter = std::make_shared<trajectory::rectangle_renderer>();
	}

	virtual_display::intersection virtual_display::intersect(const cgv::vec3 &origin,
	                                                         const cgv::vec3 &dir)
	{
		auto geom_now = display.get_geometry();
		for (auto &v : geom_now) {
			auto vm = get_model_matrix() * cgv::vec4(v, 1.0);
			v = cgv::vec3(vm.x(), vm.y(), vm.z());
		}

		auto rits = trajectory::util::ray_rect_intersection<float>::intersect_rect(
		    geom_now, origin, dir);

		auto ret = intersection{};
		ret.hit = rits.hit;
		ret.pos_rel = cgv::vec2(rits.px, rits.py);
		ret.pos_world = rits.pos;
		ret.pos_pixel = cgv::math::fvec<uint32_t, 2>(
		    static_cast<uint32_t>(static_cast<float>(resolution_x) * rits.px),
		    static_cast<uint32_t>(static_cast<float>(resolution_y) * rits.py));

		return ret;
	}

	bool virtual_display::init(cgv::render::context &ctx)
	{
		bool success = true;

		success = blitter->init(ctx);

		return success;
	}

	void virtual_display::init_frame(cgv::render::context &ctx)
	{
		blitter->init_frame(ctx); 
	}

	void virtual_display::draw(cgv::render::context &ctx)
	{
		ctx.push_modelview_matrix();
		{
			ctx.mul_modelview_matrix(get_model_matrix());
			if (draw_debug) {
				glDisable(GL_CULL_FACE);
				trajectory::util::debug::rect_t rect;
				rect.rect = display.get_geometry();
				rect.col = cgv::vec4(0.9f);
				trajectory::util::debug::draw_quad(rect);
				glEnable(GL_CULL_FACE);
			}
			else {
				glDisable(GL_CULL_FACE);
				blitter->draw(ctx);
				glEnable(GL_CULL_FACE);
			}
			ctx.pop_modelview_matrix();
		}
	}

	void virtual_display::clear(cgv::render::context &ctx)
	{ 
		blitter->clear(ctx); 
	}

	plane virtual_display::get_display() const { return display; }
	void virtual_display::set_display(const plane &display)
	{
		this->display = display;
		blitter->set_rectangle(display.get_geometry());
	}
	uint32_t virtual_display::get_resolution_x() const { return resolution_x; }
	void virtual_display::set_resolution_x(uint32_t x) { this->resolution_x = x; }
	uint32_t virtual_display::get_resolution_y() const { return resolution_y; }
	void virtual_display::set_resolution_y(uint32_t y) { this->resolution_y = y; }
	std::pair<uint32_t, uint32_t> virtual_display::get_resolution() const
	{
		return {resolution_x, resolution_y};
	}
	void virtual_display::set_resolution(uint32_t x, uint32_t y)
	{
		this->resolution_x = x;
		this->resolution_y = y;
	}
	std::weak_ptr<trajectory::rectangle_renderer> virtual_display::get_blitter() const
	{
		return blitter;
	}
	bool virtual_display::is_draw_plane() const { return draw_debug; }
	void virtual_display::set_draw_plane(bool b) { draw_debug = b; }
} // namespace room
} // namespace vr