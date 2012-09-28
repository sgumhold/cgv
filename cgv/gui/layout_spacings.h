#pragma once

#include <vector>
#include <string>

#include "lib_begin.h"

namespace cgv {
	namespace gui {

		// spacings in one direction
		struct layout_dir_spacing {
			int border;		// space from the border to the elements
			int element;	// spaces between elements
		};

		// spacings in two directions that can be named
		struct layout_spacings {
			std::string name;
			layout_dir_spacing horizontal;
			layout_dir_spacing vertical;
		};


		// get  spacings
		CGV_API const layout_spacings& get_layout_spacings(std::string name);
		// add spacings
		CGV_API void add_layout_spacings(const layout_spacings& new_spacings);
		// remove spacings
		CGV_API void remove_layout_spacings(const std::string& name);
	}
}

#include <cgv/config/lib_end.h>