#include "view.h"
#include <cgv/reflect/set_reflection_handler.h>


namespace cgv {
	namespace reflect {
		namespace render {

bool view::self_reflect(cgv::reflect::reflection_handler& rh)
{
	return 
		rh.reflect_member("focus", focus) &&
		rh.reflect_member("view_up_dir", view_up_dir) &&
		rh.reflect_member("view_dir", view_dir) &&
		rh.reflect_member("y_view_angle", y_view_angle) &&
		rh.reflect_member("y_extent_at_focus", y_extent_at_focus);
}

		}

extern_reflection_traits<cgv::render::view, cgv::reflect::render::view> get_reflection_traits(const cgv::render::view&)
{
	return extern_reflection_traits<cgv::render::view, cgv::reflect::render::view>();
}

	}
}

