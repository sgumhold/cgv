#pragma once

#include "lib_begin.h"

namespace cgv {
	namespace reflect {

			// forward declaration of reflection handler class
			class CGV_API reflection_handler;

			//! Derive from this class to announce implementation of the method self_reflect
			struct self_reflection_tag {};

	}
}

#include <cgv/config/lib_end.h>