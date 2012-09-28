#pragma once

#include <cgv/signal/signal.h>
#include <cgv/signal/abst_signal.h>

#include "lib_begin.h"

namespace cgv {
	namespace gui {

		// abstract class to define an element that can change its size
		// and position
		class CGV_API resizable {
		public:
			// get the position of the element
			virtual void get_position(int &pos_x, int &pos_y) = 0;
			// get the extents of an element
			virtual void get_extents(int &width, int &height) = 0;

			// set the position
			void set_position(int pos_x, int pos_y);

			// set the dimensions
			void set_extents(int width, int height);

			// signal other elements can connect to if they are
			// interested in size or position changes that come 
			// from the outside
			cgv::signal::signal<resizable&> extents_position_change;

		protected:
			// method for changing the size of the element
			virtual void set_extents_request(int width, int height) = 0;

			// method for changing the position of the element
			virtual void set_position_request(int pos_x, int pos_y) = 0;
		};

	}
}

#include <cgv/config/lib_end.h>