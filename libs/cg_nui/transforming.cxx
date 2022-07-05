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
			M.identity();
			iM.identity();
		}
		/// read access to model transform
		const transforming::mat4& transforming::get_model_transform() const
		{
			return M;
		}
		/// read access to inverse model transform
		const transforming::mat4& transforming::get_inverse_model_transform() const
		{
			return iM;
		}

		const render::render_types::mat4& transforming::get_global_model_transform()
		{
			mat4 accumulated_transform = get_model_transform();
			base::node* this_node = get_node();
			if (!this_node)
				return accumulated_transform;
			cgv::base::node_ptr parent = this_node->get_parent();
			int nr_iters = 0;
			do {
				if (!parent)
					break;
				auto* transforming_ptr = parent->get_interface<transforming>();
				if (transforming_ptr) {
					accumulated_transform = transforming_ptr->get_model_transform() * accumulated_transform;
				}
				parent = parent->get_parent();
			} while (++nr_iters < 100);
			return accumulated_transform;
		}

		const render::render_types::mat4& transforming::get_global_inverse_model_transform()
		{
			mat4 accumulated_transform = get_inverse_model_transform();
			base::node* this_node = get_node();
			if (!this_node)
				return accumulated_transform;
			cgv::base::node_ptr parent = this_node->get_parent();
			int nr_iters = 0;
			do {
				if (!parent)
					break;
				auto* transforming_ptr = parent->get_interface<transforming>();
				if (transforming_ptr) {
					accumulated_transform = transforming_ptr->get_inverse_model_transform() * accumulated_transform;
				}
				parent = parent->get_parent();
			} while (++nr_iters < 100);
			return accumulated_transform;
		}

		/// set model transform and compute inverse model transform
		void transforming::set_model_transform(const mat4& _M)
		{
			M = _M;
			iM = inv(M);
			base::node* obj = get_node();
			obj->on_set(&M);
			obj->on_set(&iM);
		}
		/// set model transform and inverse model transform
		void transforming::set_model_transform(const mat4& _M, const mat4& _iM)
		{
			M = _M;
			iM = _iM;
			base::node* obj = get_node();
			obj->on_set(&M);
			obj->on_set(&iM);
		}

		void transforming::set_global_model_transform(const mat4& _M)
		{
			M = get_global_inverse_model_transform() * _M;
			iM = inv(M);
			base::node* obj = get_node();
			obj->on_set(&M);
			obj->on_set(&iM);
		}

		void transforming::set_global_model_transform(const mat4& _M, const mat4& _iM)
		{
			M = get_global_inverse_model_transform() * _M;
			iM = get_global_model_transform() * _iM;
			base::node* obj = get_node();
			obj->on_set(&M);
			obj->on_set(&iM);
		}

		/// transform a point
		transforming::vec3 transforming::transform_point(const vec3& p) const
		{
			return M * vec4(p, 1.0f);
		}
		/// inverse transform a point
		transforming::vec3 transforming::inverse_transform_point(const vec3& p) const
		{
			return iM * vec4(p, 1.0f);
		}
		/// transform a vector
		transforming::vec3 transforming::transform_vector(const vec3& v) const
		{
			return M * vec4(v, 0.0f);
		}
		/// inverse transform a vector
		transforming::vec3 transforming::inverse_transform_vector(const vec3& v) const
		{
			return iM * vec4(v, 0.0f);
		}

		/// transform a normal
		transforming::vec3 transforming::transform_normal(const vec3& n) const
		{
			return vec4(n, 0.0f) * iM;
		}
		/// inverse transform a normal
		transforming::vec3 transforming::inverse_transform_normal(const vec3& n) const
		{
			return vec4(n, 0.0f) * M;
		}

		void cgv::nui::transforming::extract_transform_components(const mat4& transform, vec3& translation, quat& rotation,
			vec3& scale) const
		{
			// calculation from https://math.stackexchange.com/q/1463487 (lookup 28.06.2022)
			// assuming no perspective transformation, no shear, no negative scaling factors
			translation = transform.col(3);
			scale = vec3(vec3(transform.col(0)).length(), vec3(transform.col(1)).length(), vec3(transform.col(2)).length());
			rotation = quat(mat3({ normalize(vec3(transform.col(0))), normalize(vec3(transform.col(1))), normalize(vec3(transform.col(2))) }));
			rotation.normalize();
		}

		render::render_types::mat4 transforming::construct_transform_from_components(const vec3& translation,
			const quat& rotation, const vec3& scale) const
		{
			return  cgv::math::translate4<float>(translation) * rotation.get_homogeneous_matrix() * cgv::math::scale4<float>(scale);
		}
	}
}
#include <cgv/config/lib_end.h>