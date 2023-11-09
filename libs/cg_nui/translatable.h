#pragma once

#include <cgv/render/render_types.h>

#include "lib_begin.h"

namespace cgv {
	namespace nui {
		using namespace render;
		/// Interface for objects that have a position that can be changed.
		///	The position is represented by a 3d vector (cgv_math::vec3).
		///	The position value can either be held by the interface internally, supplied externally via a pointer or a pointer to a pointer,
		///	or be handled fully custom through overriding the get_position and set_position functions.
		///	If the implementing object extends cgv_base::base then the default implementation of set_position will call on_set with the modified value.
class CGV_API translatable
{
protected:
	vec3 internal_position{ 0.0f };
private:
	vec3* position_ptr{ nullptr };
	vec3** position_ptr_ptr{ nullptr };

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
	/// Use internal position variable or custom override of get_position and set_position
	translatable() {}
	/// Use given external position variable
	translatable(vec3* position_ptr) : position_ptr(position_ptr) {}
	/// Use given external position variable through one indirection
	translatable(vec3** position_ptr_ptr) : position_ptr_ptr(position_ptr_ptr) {}

	virtual vec3 get_position() const
	{
		if (position_ptr_ptr)
			return **position_ptr_ptr;
		if (position_ptr)
			return *position_ptr;
		return internal_position;
	}

	virtual void set_position(const vec3& position)
	{
		if (position_ptr_ptr) {
			**position_ptr_ptr = position;
			if (get_base())
				_base->on_set(*position_ptr_ptr);
		}
		else if (position_ptr) {
			*position_ptr = position;
			if (get_base())
				_base->on_set(position_ptr);
		}
		else {
			internal_position = position;
		}
	}
};

	}
}

#include <cgv/config/lib_end.h>