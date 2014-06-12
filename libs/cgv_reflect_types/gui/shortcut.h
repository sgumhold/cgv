#pragma once

#include <cgv/gui/shortcut.h>
#include <cgv/reflect/reflect_extern.h>

#include <cgv_reflect_types/lib_begin.h>

namespace cgv {
	namespace reflect {
		namespace gui {

struct CGV_API shortcut : public cgv::gui::shortcut
{
	bool self_reflect(cgv::reflect::reflection_handler& rh);
};

		}

#ifdef REFLECT_IN_CLASS_NAMESPACE
}} namespace cgv { namespace shortcut {
#endif

extern CGV_API cgv::reflect::extern_reflection_traits<cgv::gui::shortcut, cgv::reflect::gui::shortcut> get_reflection_traits(const cgv::gui::shortcut&);

	}
}

#include <cgv/config/lib_end.h>
