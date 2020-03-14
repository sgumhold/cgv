#pragma once

#include <cgv/math/fvec.h>
#include "rectangle_renderer.h"

#include "placeable.h"
#include "plane.h"

using namespace cgv::render;

namespace vr {
namespace room {
	// simple wrapper for drawable and placable
	class virtual_display : public drawable, public placeable 
	{
	  public:
		struct intersection {
			vec3 pos_world;
			vec2 pos_rel;
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

		intersection intersect(const vec3 &origin, const vec3 &dir);

		bool init(context &ctx);
		void init_frame(context &ctx);

		void draw(context &ctx);

		void clear(context &ctx);

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