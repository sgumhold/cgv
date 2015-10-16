#include "shortcut.h"
#include <cgv/reflect/set_reflection_handler.h>
#include <cgv/utils/convert_string.h>

namespace cgv {
	namespace reflect {
		namespace gui {

bool shortcut::self_reflect(cgv::reflect::reflection_handler& rh)
{
	bool res;
	std::string text;
	if (rh.is_creative()) {
		res = rh.reflect_member("shortcut", text);
		cgv::utils::from_string(*this, text);
	}
	else {
		text = cgv::utils::to_string(*this);
		res = rh.reflect_member("shortcut", text);
	}
	return res;
}

		}

#ifdef REFLECT_IN_CLASS_NAMESPACE
}} namespace cgv { namespace gui {
#endif

cgv::reflect::extern_reflection_traits<cgv::gui::shortcut, cgv::reflect::gui::shortcut> get_reflection_traits(const cgv::gui::shortcut&)
{
	return cgv::reflect::extern_reflection_traits<cgv::gui::shortcut, cgv::reflect::gui::shortcut>();
}

	}
}

