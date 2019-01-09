#include "color_storage.h"

namespace cgv {
	namespace media {

		abst_color_storage::abst_color_storage(ColorType _color_type)
			: color_type(_color_type)
		{
		}

		abst_color_storage::~abst_color_storage()
		{
		}

		abst_color_storage::ColorType abst_color_storage::get_color_type() const
		{
			return color_type;
		}

		/// return size of a single color in byte
		size_t abst_color_storage::get_color_size() const
		{
			size_t sizes[] = { sizeof(rgb),sizeof(rgba), sizeof(rgb8), sizeof(rgba8) };
			return sizes[color_type];
		}
	}
}