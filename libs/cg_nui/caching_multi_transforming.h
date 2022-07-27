#pragma once

#include <cg_nui/transforming.h>

#include "lib_begin.h"

namespace cgv {
	namespace nui {

/// Transforming interface with internal storage for transforms of several subobjects relative to the main object.
///	Can be used as an implementation of transforming as is.
///	Used to hide the existence of subobjects handled through primitive indices from classes relying on the transforming interface.
class CGV_API caching_multi_transforming : public transforming
{
protected:
	/// store model and inverse model matrix of object
	mat4 M, iM;
	/// Store model and inverse model matrices of sub-objects
	std::vector<mat4> sub_matrices, inverse_sub_matrices;
	/// Currently active sub-object (primitive index).
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
	caching_multi_transforming() {}
	/// get currently active (sub-)object primitive index
	int get_active_object() const;

	/// read access to model transform (local)
	const mat4& get_model_transform() const override;
	/// read access to inverse model transform (local)
	const mat4& get_inverse_model_transform() const override;

	/// set model transform and compute inverse model transform (local)
	void set_model_transform(const mat4& _M) override;
	/// set model transform and inverse model transform (local)
	void set_model_transform(const mat4& _M, const mat4& _iM) override;
};

	}
}
#include <cgv/config/lib_end.h>