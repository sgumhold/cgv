#pragma once

#include <cg_nui/transforming.h>

#include "lib_begin.h"

namespace cgv {
	namespace nui {

/// interface for objects that provide multiple modelview_projection_window_matrices of several subobjects relative to the main object
///	Used to hide the existence of subobjects handled through primitive indices from classes relying on the transforming interface (e.g. posable or gizmos).
class CGV_API multi_transforming : public transforming
{
	base::node* _node;
	bool tried_node_cast{ false };
	base::node* get_node();
protected:
	/// store model and inverse model matrices of sub-objects
	std::vector<mat4> sub_matrices, inverse_sub_matrices;
	/// currently active sub-object (primitive index)
	///	Main object active if -1.
	int active_object = { -1 };

	/// set currently active (sub-)object by primitive index
	void set_active_object(int primitive_index);
	/// add a transform for a new sub object to the end of the list
	///	Returns the primitive index of the added transform.
	int add_sub_object_transform(const mat4& transform);
	/// add a transform for a new sub object to at the specified index
	///	Returns the primitive index of the added transform.
	int add_sub_object_transform(const mat4& transform, int index);
	/// remove the last sub object transform
	///	Adjusts the currently active object if necessary.
	void remove_sub_object_transform();
	/// remove the sub object transform at the specified index
	///	Adjusts the currently active object if necessary.
	void remove_sub_object_transform(int index);
public:
	multi_transforming() {}
	/// get currently active (sub-)object primitive index
	int get_active_object() const;
	/// read access to model transform (local)
	const mat4& get_model_transform() const override;
	/// read access to inverse model transform (local)
	const mat4& get_inverse_model_transform() const override;
	/// read access to accumulated model transform (global)
	const mat4& get_global_model_transform() override;
	/// read access to accumulated inverse model transform (global)
	const mat4& get_global_inverse_model_transform() override;
	/// set model transform and compute inverse model transform (local)
	void set_model_transform(const mat4& _M) override;
	/// set model transform and inverse model transform (local)
	void set_model_transform(const mat4& _M, const mat4& _iM) override;
	/// set model transform and compute inverse model transform (global)
	///	Computes local transform such that the accumulated global transform matches the given transform
	void set_global_model_transform(const mat4& _M) override;
	///	set model transform and inverse model transform (global)
	///	Computes local transform such that the accumulated global transform matches the given transform
	void set_global_model_transform(const mat4& _M, const mat4& _iM) override;
	/// transform a point
	vec3 transform_point(const vec3& p) const override;
	/// inverse transform a point
	vec3 inverse_transform_point(const vec3& p) const override;
	/// transform a vector
	vec3 transform_vector(const vec3& v) const override;
	/// inverse transform a vector
	vec3 inverse_transform_vector(const vec3& v) const override;
	/// transform a normal
	vec3 transform_normal(const vec3& n) const override;
	/// inverse transform a normal
	vec3 inverse_transform_normal(const vec3& n) const override;
};

	}
}
#include <cgv/config/lib_end.h>