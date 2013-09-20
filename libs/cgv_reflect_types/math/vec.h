#pragma once

#include <cgv/math/vec.h>
#include <cgv/reflect/reflect_extern.h>
#include <cgv/reflect/reflection_handler.h>

namespace cgv {
	namespace reflect {
		namespace math {

template <typename T>
struct vec : public cgv::math::vec<T>
{
	bool self_reflect(cgv::reflect::reflection_handler& rh) {
		return 
			rh.reflect_member("data_is_external", this->data_is_external) &&
			rh.reflect_array("coords", this->_data, this->_size);
	}
};
		}
	}
}
namespace cgv {
	namespace math {

		template <typename T>
		cgv::reflect::extern_string_reflection_traits<vec<T>, cgv::reflect::math::vec<T> > 
			get_reflection_traits(const vec<T>&) { 
				return cgv::reflect::extern_string_reflection_traits<vec<T>, cgv::reflect::math::vec<T> >(); 
		}

	}
}
