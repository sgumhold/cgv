#include "plane.h"

namespace vr {
namespace room {
	plane::plane() : geometry({vec3{0.0f}, vec3{0.0f}, vec3{0.0f}, vec3{0.0f}}) {}
	plane::plane(const rectangle &geometry) : geometry(geometry) {}

	rectangle plane::get_geometry() const { return geometry; }
	void plane::generate_geometry_xy(float width, float height, bool reverse)
	{
		width *= 0.5f;
		height *= 0.5f;

		rectangle r = {vec3{-width, height, 0.0f}, vec3{width, height, 0.0f},
		               vec3{-width, -height, 0.0f}, vec3{width, -height, 0.0f}};
		if (reverse) {
			std::swap(r[0], r[1]);
			std::swap(r[2], r[3]);
		}

		geometry = r;
	}
	void plane::generate_geometry_xz(float length, float width, bool reverse)
	{
		length *= 0.5f;
		width *= 0.5f;

		rectangle r = {vec3{-width, 0.0f, -length}, vec3{width, 0.0f, -length},
		               vec3{-width, 0.0f, length}, vec3{width, 0.0f, length}};
		if (reverse) {
			std::swap(r[0], r[1]);
			std::swap(r[2], r[3]);
		}

		geometry = r;
	}
	void plane::generate_geometry_yz(float length, float height, bool reverse)
	{
		length *= 0.5f;
		height *= 0.5f;

		rectangle r = {vec3{0.0f, height, length}, vec3{0.0f, height, -length},
		               vec3{0.0f, -height, length}, vec3{0.0f, -height, -length}};
		if (reverse) {
			std::swap(r[0], r[1]);
			std::swap(r[2], r[3]);
		}

		geometry = r;
	}
	void plane::set_geometry(const rectangle &geometry) { this->geometry = geometry; }

} // namespace room
} // namespace vr