#pragma once

#include "placeable.h"
#include "rectangle.h"

#include "ray_rect_intersection.h"

using namespace cgv::data;
using namespace cgv::render;

namespace vr {
namespace room {

	class plane : public placeable {
	  protected:
		rectangle geometry;

	  public:
		plane();
		plane(const rectangle &geometry);

		rectangle get_geometry() const;
		void plane::set_geometry(const rectangle &geometry);

		// e.g. front, back
		void generate_geometry_xy(float width, float height, bool reverse = false);
		// e.g. floor, ceiling
		void generate_geometry_xz(float length, float width, bool reverse = false);
		// e.g. left, right
		void generate_geometry_yz(float length, float height, bool reverse = false);

		/*void set_ground(const rect &geometry);
		void set_ground_square(float meter = 1000.0f, bool reverse = false);*/
	};
} // namespace room
} // namespace vr