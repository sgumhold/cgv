#pragma once

#include <cgv/gui/layout.h>

#include "lib_begin.h"

namespace cgv {
	namespace gui {

		// layout to arrange all children in an inline alignment
		class CGV_API layout_inline: public layout
		{
		public:
			layout_inline();
			layout_inline(cgv::base::group_ptr container);

			~layout_inline();

			// the core update function to align the elements inline
			void update();
		};


/// ref counted pointer to table layout
typedef cgv::data::ref_ptr<layout_inline> layout_inline_ptr;


	}
}


#include <cgv/config/lib_end.h>