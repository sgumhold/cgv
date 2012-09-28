#include "fltk_gui_group.h"
#include <cgv/type/variant.h>

#ifdef WIN32
#pragma warning(disable:4311)
#endif
#include <fltk/Widget.h>
#include <fltk/Group.h>
#ifdef WIN32
#pragma warning(default:4311)
#endif

using namespace cgv::type;

/** called by the driver before adding a new element to the group.  
    The group generates dimensions which are passed over to the 
	 constructor of the newly created element. */
void fltk_gui_group::prepare_new_element(cgv::gui::gui_group_ptr ggp, int& x, int& y, int& w, int& h)
{
	x = 0;
	y = 0;
	w = 200;
	h = 100;
}

/// called by the driver after a new element has been constructed
void fltk_gui_group::finalize_new_element(cgv::gui::gui_group_ptr ggp, const std::string& , base_ptr )
{
}

/// remove the given child, if it appears several times, remove al instances. Return the number of removed children
unsigned int fltk_gui_group::remove_child(cgv::gui::gui_group_ptr ggp, base_ptr child)
{
	fltk::Group* g = static_cast<fltk::Group*>(
		static_cast<fltk::Widget*>(ggp->get_user_data()));

	unsigned int n = g->children();
	unsigned int count = 0;
	for (unsigned int i=0; i<n; ++i) {
		base_ptr c(static_cast<cgv::base::base*>(g->child(i)->user_data()));
		if (child == c) {
			g->remove(i);
			--n;
			--i;
			++count;
		}
	}
	return ggp->cgv::gui::gui_group::remove_child(child);
}

/// remove all children
void fltk_gui_group::remove_all_children(cgv::gui::gui_group_ptr ggp)
{
	fltk::Group* g = static_cast<fltk::Group*>(
		static_cast<fltk::Widget*>(ggp->get_user_data()));
	g->remove_all();
	return ggp->cgv::gui::gui_group::remove_all_children();
}
