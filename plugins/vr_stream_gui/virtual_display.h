#pragma once

#include <cgv/math/fvec.h>
#include "rectangle_renderer.h"

#include "placeable.h"
#include "plane.h"

namespace vr {
namespace room {
	// simple wrapper for drawable and placable
	class virtual_display : public cgv::render::drawable, public cgv::render::placeable
	{
	  public:
		struct intersection {
			cgv::vec3 pos_world;
			cgv::vec2 pos_rel;
			cgv::math::fvec<uint32_t, 2> pos_pixel;
			bool hit;
		};

	  private:
		plane display;
		uint32_t resolution_x = 1920u;
		uint32_t resolution_y = 1080u;
		bool draw_debug = false;

		std::shared_ptr<trajectory::rectangle_renderer> blitter;

		// TODO: 3d model
	  public:
		virtual_display();
		virtual_display(const plane &display);

		intersection intersect(const cgv::vec3 &origin, const cgv::vec3 &dir);

		bool init(cgv::render::context &ctx);
		void init_frame(cgv::render::context &ctx);

		void draw(cgv::render::context &ctx);

		void clear(cgv::render::context &ctx);

		plane get_display() const;
		void set_display(const plane &display);

		uint32_t get_resolution_x() const;
		void set_resolution_x(uint32_t x);
		uint32_t get_resolution_y() const;
		void set_resolution_y(uint32_t y);
		std::pair<uint32_t, uint32_t> get_resolution() const;
		void set_resolution(uint32_t x, uint32_t y);

		std::weak_ptr<trajectory::rectangle_renderer> get_blitter() const;

		bool is_draw_plane() const;
		void set_draw_plane(bool b);
	};
} // namespace room
} // namespace vr