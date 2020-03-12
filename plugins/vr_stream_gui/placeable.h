#pragma once

#include <cgv/math/ftransform.h>
#include <cgv/render/render_types.h>

namespace cgv {
namespace render {

	class placeable : public render_types {
	  public:
		enum class rotation_order : uint8_t {
			XYZ,
			XZY,
			YXZ,
			YZX,
			ZXY,
			ZYX // blender, glm
		};

	  private:
		vec3 position = vec3(0.0f);
		vec3 rotation = vec3(0.0f);
		vec3 scale = vec3(1.0f);
		mat4 model_matrix = cgv::math::identity4<float>();

		vec3 forward = vec3(0.0f, 0.0f, 1.0f);
		vec3 right = vec3(1.0f, 0.0f, 0.0f);
		vec3 up = vec3(0.0f, 1.0f, 0.0f);

		rotation_order rot_order = rotation_order::ZYX;

		void calculate_model_matrix();

	  public:
		placeable();
		placeable(const vec3 &position);
		placeable(const vec3 &position, const vec3 &otation);
		placeable(const vec3 &position, const vec3 &rotation, const vec3 &scale);

		vec3 get_position() const;
		void set_position(const vec3 &position);
		void set_position_x(const float x);
		void set_position_y(const float y);
		void set_position_z(const float z);

		vec3 get_rotation() const;
		void set_rotation(const vec3 &xyz);
		void set_rotation_x(const float x);
		void set_rotation_y(const float y);
		void set_rotation_z(const float z);

		vec3 get_scale();
		void set_scale(const vec3 &scale);
		void set_scale_x(const float x);
		void set_scale_y(const float y);
		void set_scale_z(const float z);

		mat4 get_model_matrix() const;
		void set_model_matrix(const mat4 &model_matrix);

		void set_rotation_order(rotation_order order);
	};

} // namespace render
} // namespace cgv
