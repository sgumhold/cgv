#include "transforming.h"
#include <cgv/math/inv.h>
#include <cgv/base/node.h>
#include <cgv/math/ftransform.h>

namespace cgv {
	namespace nui {

		base::node* transforming::get_node()
		{
			if (!tried_node_cast) {
				_node = dynamic_cast<base::node*>(this);
				tried_node_cast = true;
				if (!_node)
					throw std::exception("Objects implementing 'transforming' have to inherit 'base::node'");
			}
			return _node;
		}

		transforming::transforming()
		{

		}

		const mat4& transforming::get_inverse_model_transform() const
		{
			return inv(get_model_transform());
		}

		vec3 transforming::get_local_position() const
		{
			// calculation from https://math.stackexchange.com/q/1463487 (lookup 28.06.2022)
			// assuming no perspective transformation, no shear, no negative scaling factors
			return get_model_transform().col(3);
		}

		quat transforming::get_local_rotation() const
		{
			// calculation from https://math.stackexchange.com/q/1463487 (lookup 28.06.2022)
			// assuming no perspective transformation, no shear, no negative scaling factors
			mat4 transform = get_model_transform();
			quat rotation = quat(mat3({ normalize(vec3(transform.col(0))), normalize(vec3(transform.col(1))), normalize(vec3(transform.col(2))) }));
			rotation.normalize();
			return rotation;
		}

		vec3 transforming::get_local_scale() const
		{
			// calculation from https://math.stackexchange.com/q/1463487 (lookup 28.06.2022)
			// assuming no perspective transformation, no shear, no negative scaling factors
			mat4 transform = get_model_transform();
			return vec3(vec3(transform.col(0)).length(), vec3(transform.col(1)).length(), vec3(transform.col(2)).length());
		}

		void transforming::set_global_model_transform(const mat4& _M)
		{
			if (get_node())
				set_model_transform(get_global_inverse_model_transform(_node) * _M);
			else
				set_model_transform(_M);
		}

		void transforming::set_global_model_transform(const mat4& _M, const mat4& _iM)
		{
			if (get_node())
				set_model_transform(get_global_inverse_model_transform(_node) * _M, get_global_model_transform(_node) * _iM);
			else
				set_model_transform(_M, _iM);
		}

		/// transform a point with the local model transform
		vec3 transforming::transform_point(const vec3& p) const
		{
			return  get_model_transform() * vec4(p, 1.0f);
		}
		/// inverse transform a point with the local model transform
		vec3 transforming::inverse_transform_point(const vec3& p) const
		{
			return get_inverse_model_transform() * vec4(p, 1.0f);
		}
		/// transform a vector with the local model transform
		vec3 transforming::transform_vector(const vec3& v) const
		{
			return get_model_transform() * vec4(v, 0.0f);
		}
		/// inverse transform a vector with the local model transform
		vec3 transforming::inverse_transform_vector(const vec3& v) const
		{
			return get_inverse_model_transform() * vec4(v, 0.0f);
		}

		/// transform a normal with the local model transform
		vec3 transforming::transform_normal(const vec3& n) const
		{
			return vec4(n, 0.0f) * get_inverse_model_transform();
		}
		/// inverse transform a normal with the local model transform
		vec3 transforming::inverse_transform_normal(const vec3& n) const
		{
			return vec4(n, 0.0f) * get_model_transform();
		}

		const mat4& transforming::get_global_model_transform(base::node_ptr obj)
		{
			mat4& accumulated_transform = *(new mat4());
			accumulated_transform.identity();
			base::node_ptr parent = obj;
			int nr_iters = 0;
			do {
				auto* transforming_ptr = parent->get_interface<transforming>();
				if (transforming_ptr) {
					accumulated_transform = transforming_ptr->get_model_transform() * accumulated_transform;
				}
				parent = parent->get_parent();
			} while (parent && ++nr_iters < 100);
			return accumulated_transform;
		}

		const mat4& transforming::get_global_inverse_model_transform(base::node_ptr obj)
		{
			mat4& accumulated_transform = *(new mat4());
			accumulated_transform.identity();
			base::node_ptr parent = obj;
			int nr_iters = 0;
			do {
				auto* transforming_ptr = parent->get_interface<transforming>();
				if (transforming_ptr) {
					accumulated_transform = transforming_ptr->get_inverse_model_transform() * accumulated_transform;
				}
				parent = parent->get_parent();
			} while (parent && ++nr_iters < 100);
			return accumulated_transform;
		}

		void cgv::nui::transforming::extract_transform_components(const mat4& transform, vec3& translation, quat& rotation,
			vec3& scale)
		{
			// calculation from https://math.stackexchange.com/q/1463487 (lookup 28.06.2022)
			// assuming no perspective transformation, no shear, no negative scaling factors
			translation = transform.col(3);
			scale = vec3(vec3(transform.col(0)).length(), vec3(transform.col(1)).length(), vec3(transform.col(2)).length());
			rotation = quat(mat3({ normalize(vec3(transform.col(0))), normalize(vec3(transform.col(1))), normalize(vec3(transform.col(2))) }));
			rotation.normalize();
		}

		const mat4& transforming::construct_transform_from_components(const vec3& translation,
			const quat& rotation, const vec3& scale)
		{
			mat4& transform = *(new mat4());
			transform = cgv::math::translate4<float>(translation) * rotation.get_homogeneous_matrix() * cgv::math::scale4<float>(scale);
			return transform;
		}

		const mat4& transforming::construct_inverse_transform_from_components(const vec3& inverse_translation,
			const quat& inverse_rotation, const vec3& inverse_scale)
		{
			mat4& transform = *(new mat4());
			transform = cgv::math::scale4<float>(inverse_scale) * inverse_rotation.get_homogeneous_matrix() * cgv::math::translate4<float>(inverse_translation);
			return transform;
		}
	}
}
#include <cgv/config/lib_end.h>