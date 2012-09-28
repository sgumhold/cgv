#pragma once

namespace cgv {
	namespace type {
		namespace traits {
			/** the zero traits defines for each type in the static const member \c value, 
			    what the zero value is. */
			template <typename T> struct zero { static const T value = 0; };
		}
	}
}