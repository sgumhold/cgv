#pragma once

#include "fltk_base.h"
#include <cgv/gui/gui_group.h>

namespace fltk {
	class Widget;
	class Group;
}


#include "lib_begin.h"

/** the fltk_gui_group implements the common interface of gui_group
    and extends the interface by methods to handle the incorporation
	 of new elements. */
struct CGV_API fltk_gui_group : public fltk_base
{
	/** called by the driver before adding a new element to the group.  
	    The group generates dimensions which are passed over to the 
		 constructor of the newly created element. */
	virtual void prepare_new_element(cgv::gui::gui_group_ptr ggp, int& x, int& y, int& w, int& h);
	/// called by the driver after a new element has been constructed
	virtual void finalize_new_element(cgv::gui::gui_group_ptr ggp, const std::string& align, base_ptr child);
	/// remove the given child, if it appears several times, remove al instances. Return the number of removed children
	virtual unsigned int remove_child(cgv::gui::gui_group_ptr ggp, base_ptr child);
	/// remove all children
	virtual void remove_all_children(cgv::gui::gui_group_ptr ggp);
};

#include <cgv/config/lib_end.h>