#pragma once

#include <cgv/math/ftransform.h>

namespace cgv {
namespace render {

	class placeable
	{
	  protected:
		vec3 position;
		quat orientation;
		vec3 scale;
	  public:
		placeable();
		placeable(const vec3 &_position);
		placeable(const vec3 &_position, const quat& _orientation);
		placeable(const vec3 &_position, const quat& _orientation, const vec3 &_scale);

		const vec3& get_position() const { return position; }
		vec3& ref_position() { return position; }
		void set_position(const vec3& _position) { position = _position; }
		const quat& get_orientation() const { return orientation; }
		quat& ref_orientation() { return orientation; }
		void set_orientation(const quat& _orientation) { orientation = _orientation;  }
		const vec3& get_scale() const { return scale; }
		vec3& ref_scale() { return scale; }
		void set_scale(const vec3& _scale) { scale = _scale; }

		mat4 get_model_matrix() const;
	};

} // namespace render
} // namespace cgv
