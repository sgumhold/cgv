#include <cgv/gui/resizable.h>

namespace cgv {
	namespace gui {

		void resizable::set_extents(int width, int height)
		{
			// ask the actual element to change its size
			set_extents_request(width, height);

			// inform everyone who is interested
			extents_position_change(*this);
		}


		void resizable::set_position(int pos_x, int pos_y)
		{
			// ask the actual element to change its position
			set_position_request(pos_x, pos_y);

			// inform everyone who is interested
			extents_position_change(*this);
		}

	}
}