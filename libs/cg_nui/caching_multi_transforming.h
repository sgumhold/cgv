#pragma once

#include <cg_nui/transforming.h>

#include "lib_begin.h"

namespace cgv {
	namespace nui {
// TODO: Split into two subversion? (Currently variant A is used)
// Variant A: Store sub-object transform as is (main object transform and sub-object transform already combined). The parent-child relationship
// is only enforced when the main object transform is changed. This way there are no additional calculations when accessing or changing the sub-object
// transform from the outside. This variant is better performance wise if the sub-object transforms are accessed and changed more frequently than
// the main object transform (or if only a small subset of the sub-objects is used much more frequently).
// Variant B: Store sub-object transforms relative to main object. Transforms have to be combined again every time before access from the outside.
// This requires fewer calculations if the main object transform is generally changed often compared to the sub-object and the sub-objects also get
// get accessed fairly uniformly.

/// Transforming interface with internal storage for transforms of several subobjects relative to a main object.
///	The subobject transforms are relative to the main object internally, but will be properly combined when accessed through the interface.
///	Can be used as an implementation of transforming as is.
///	Used to hide the existence of subobjects handled through primitive indices from classes relying on the transforming interface.
class CGV_API caching_multi_transforming : public transforming
{
protected:
	/// Internally stored model matrix of the main object
	mat4 M;
	/// Internally stored inverse model matrix of the main object
	mat4 iM;
	/// Internally stored model matrices of sub-objects
	std::vector<mat4> sub_matrices;
	/// Internally stored inverse model matrices of subobjects
	std::vector<mat4> inverse_sub_matrices;
	/// Currently active sub-object (primitive index).
	///	Main object active if -1.
	int active_object = { -1 };

	/// Set currently active (sub)object by primitive index
	void set_active_object(int primitive_index);
	/// Add a transform for a new sub-object to the end of the list.
	///	Returns the primitive index of the added transform.
	///	Variant A: Assumes transform is already combined with the main object transform
	///	Variant B: Assumes transform is relative to the main object transform
	int add_sub_object_transform(const mat4& transform);
	/// Add a transform for a new sub-object to at the specified index.
	///	Returns the primitive index of the added transform.
	///	(This obviously shifts the assignments of indices to sub-objects.)
	///	Variant A: Assumes transform is already combined with the main object transform
	///	Variant B: Assumes transform is relative to the main object transform
	int add_sub_object_transform(const mat4& transform, int index);
	/// Remove the last sub-object transform.
	///	Adjusts the currently active object if necessary.
	void remove_sub_object_transform();
	/// Remove the sub object transform at the specified index.
	///	Adjusts the currently active object if necessary.
	///	(This shifts the assignments of indices to sub-objects.)
	void remove_sub_object_transform(int index);
public:
	caching_multi_transforming() {}
	/// Get currently active (sub)object primitive index
	int get_active_object() const;

	// Implementation of the transforming interface using the internally stored model matrix of the currently active (sub)object

	mat4 get_model_transform() const override;
	mat4 get_inverse_model_transform() const override;
	void set_model_transform(const mat4& _M) override;
	void set_model_transform(const mat4& _M, const mat4& _iM) override;
};

	}
}
#include <cgv/config/lib_end.h>