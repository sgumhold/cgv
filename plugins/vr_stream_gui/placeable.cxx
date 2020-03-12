#include <cgv/math/ftransform.h>
#include <cgv/math/quaternion.h>
#include <cgv/math/quat.h>

#include "placeable.h"


namespace cgv {
namespace render {

	// could use delegate-constructors
	placeable::placeable()
	{
		// also possible: model_matrix.identity();
	}
	placeable::placeable(const vec3 &position)
	    : position(position)
	{
		calculate_model_matrix();
	}
	placeable::placeable(const vec3 &position, const vec3 &rotation)
	    : position(position), rotation(rotation)
	{
		calculate_model_matrix();
	}

	placeable::placeable(const vec3 &position, const vec3 &rotation,
	                     const vec3 &scale)
	    : position(position), rotation(rotation), scale(scale)
	{
		calculate_model_matrix();
	}

	void placeable::calculate_model_matrix()
	{
		auto x = cgv::math::rotate4(rotation.x(), right);
		auto y = cgv::math::rotate4(rotation.y(), up);
		auto z = cgv::math::rotate4(rotation.z(), forward);

		mat4 rot;
		if(rot_order == rotation_order::XYZ) rot = x * y * z;
		else if (rot_order == rotation_order::XZY) rot = x * z * y;
		else if (rot_order == rotation_order::YXZ) rot = y * x * z;
		else if (rot_order == rotation_order::YZX) rot = y * z * x;
		else if (rot_order == rotation_order::ZXY) rot = z * x * y;
		else if (rot_order == rotation_order::ZYX) rot = z * y * x;

		/*cgv::math::quaternion<float> qx(right, rotation.x());
		cgv::math::quaternion<float> qy(up, rotation.y());
		cgv::math::quaternion<float> qz(-forward, rotation.z());

		auto qr = qx * qy * qz;

		mat3 rot3;
		rot3.identity();
		qr.rotate(rot3);

		mat4 rot4 = cgv::math::identity4<float>();
		for (int j = 0; j < 3; ++j) {
			for (int i = 0; i < 3; ++i) {
				rot4(i,j) = rot3(i,j);
			}
		}*/

		//vec3 angle = (rotation / 360.0f);
		//float rot_angle = std::acos(cgv::math::dot(vec3(0.0, 0.0, -1.0), angle));
		//vec3 rot_axis = cgv::math::cross(vec3(0.0, 0.0, 1.0), angle);
		//rot_axis.normalize();
		//rot = cgv::math::rotate4(rot_angle, rot_axis);

		model_matrix = cgv::math::translate4(position) * rot * cgv::math::scale4(scale);
	}
	placeable::vec3 placeable::get_position() const { return position; }
	void placeable::set_position(const vec3 &position)
	{
		this->position = position;
		calculate_model_matrix();
	}
	void placeable::set_position_x(const float x)
	{
		position.x() = x;
		calculate_model_matrix();
	}
	void placeable::set_position_y(const float y)
	{
		position.y() = y;
		calculate_model_matrix();
	}
	void placeable::set_position_z(const float z)
	{
		position.z() = z;
		calculate_model_matrix();
	}
	placeable::vec3 placeable::get_rotation() const { return rotation; }
	void placeable::set_rotation(const vec3 &pyr)
	{
		rotation = pyr;
		calculate_model_matrix();
	}
	void placeable::set_rotation_x(const float x)
	{
		rotation.x() = std::fmod(x,360.0f);
		calculate_model_matrix();
	}
	void placeable::set_rotation_y(const float y)
	{
		rotation.y() = std::fmod(y, 360.0f);
		calculate_model_matrix();
	}
	void placeable::set_rotation_z(const float z)
	{
		rotation.z() = std::fmod(z, 360.0f);
		calculate_model_matrix();
	}
	placeable::vec3 placeable::get_scale() { return scale; }
	void placeable::set_scale(const vec3 &scale)
	{
		this->scale = scale;
		calculate_model_matrix();
	}
	void placeable::set_scale_x(const float x)
	{
		scale.x() = x;
		calculate_model_matrix();
	}
	void placeable::set_scale_y(const float y)
	{
		scale.y() = y;
		calculate_model_matrix();
	}
	void placeable::set_scale_z(const float z)
	{
		scale.z() = z;
		calculate_model_matrix();
	}
	placeable::mat4 placeable::get_model_matrix() const { return model_matrix; }
	void placeable::set_model_matrix(const mat4 &model_matrix)
	{
		this->model_matrix = model_matrix;
		calculate_model_matrix();
	}

	void placeable::set_rotation_order(rotation_order order)
	{
		rot_order = order;
	}

} // namespace render
} // namespace cgv