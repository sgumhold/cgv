#pragma once

#include <cgv/render/render_types.h>

#include "lib_begin.h"

namespace cgv {
	namespace nui {
		using namespace render;
/// Interface for objects that have a rotation/orientation that can be changed.
///	The rotation/orientation is represented by a Quaternion (cgv_math::quat).
///	The rotation value can either be held by the interface internally, supplied externally via a pointer or a pointer to a pointer,
///	or be handled fully custom through overriding the get_rotation and set_rotation functions.
///	If the implementing object extends cgv_base::base then the default implementation of set_rotation will call on_set with the modified value.
class CGV_API rotatable
{
protected:
	quat internal_rotation{ };
private:
	quat* rotation_ptr{ nullptr };
	quat** rotation_ptr_ptr{ nullptr };

	base::base* _base{ nullptr };
	bool tried_base_cast{ false };
protected:
	base::base* get_base()
	{
		if (!_base && !tried_base_cast) {
			_base = dynamic_cast<base::base*>(this);
			tried_base_cast = true;
		}
		return _base;
	};
public:
	/// Use internal rotation variable or custom override of get_rotation and set_rotation
	rotatable() {}
	/// Use given external rotation variable
	rotatable(quat* rotation_ptr) : rotation_ptr(rotation_ptr) {}
	/// Use given external rotation variable through one indirection
	rotatable(quat** rotation_ptr_ptr) : rotation_ptr_ptr(rotation_ptr_ptr) {}

	virtual quat get_rotation() const
	{
		if (rotation_ptr_ptr)
			return **rotation_ptr_ptr;
		if (rotation_ptr)
			return *rotation_ptr;
		return internal_rotation;
	}

	virtual void set_rotation(const quat& rotation)
	{
		if (rotation_ptr_ptr) {
			**rotation_ptr_ptr = rotation;
			if (get_base())
				_base->on_set(*rotation_ptr_ptr);
		}
		else if (rotation_ptr) {
			*rotation_ptr = rotation;
			if (get_base())
				_base->on_set(rotation_ptr);
		}
		else {
			internal_rotation = rotation;
			if (get_base())
				_base->on_set(&internal_rotation);
		}
	}
};

	}
}

#include <cgv/config/lib_end.h>