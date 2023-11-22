#pragma once

#include <cgv/render/render_types.h>

#include "lib_begin.h"

namespace cgv {
	namespace nui {
		using namespace render;
/// Interface for objects that have a scale that can be changed.
///	The scale is represented by a 3d vector (cgv_math::vec3).
///	The scale value can either be held by the interface internally, supplied externally via a pointer or a pointer to a pointer,
///	or be handled fully custom through overriding the get_scale and set_scale functions.
///	If the implementing object extends cgv_base::base then the default implementation of set_scale will call on_set with the modified value.
class CGV_API scalable
{
protected:
	vec3 internal_scale{ 0.0f };
private:
	vec3* scale_ptr{ nullptr };
	vec3** scale_ptr_ptr{ nullptr };

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
	/// Use internal scale variable or custom override of get_scale and set_scale
	scalable() {}
	/// Use given external scale variable
	scalable(vec3* scale_ptr) : scale_ptr(scale_ptr) {}
	/// Use given external scale variable through one indirection
	scalable(vec3** scale_ptr_ptr) : scale_ptr_ptr(scale_ptr_ptr) {}

	virtual vec3 get_scale() const
	{
		if (scale_ptr_ptr)
			return **scale_ptr_ptr;
		if (scale_ptr)
			return *scale_ptr;
		return internal_scale;
	}

	virtual void set_scale(const vec3& scale)
	{
		if (scale_ptr_ptr) {
			**scale_ptr_ptr = scale;
			if (get_base())
				_base->on_set(*scale_ptr_ptr);
		}
		else if (scale_ptr) {
			*scale_ptr = scale;
			if (get_base())
				_base->on_set(scale_ptr);
		}
		else {
			internal_scale = scale;
			if (get_base())
				_base->on_set(&internal_scale);
		}
	}
};

	}
}

#include <cgv/config/lib_end.h>