#include "transformed.h"

namespace cgv {
	namespace nui {

		transformed::transformed()
		{
			MVPW.identity();
		}
		void transformed::set_modelview_projection_window_matrix(const cgv::render::context& ctx)
		{
			MVPW = ctx.get_modelview_projection_window_matrix();
		}
		const dmat4& transformed::get_modelview_projection_window_matrix() const
		{
			return MVPW;
		}

	}
}
#include <cgv/config/lib_end.h>