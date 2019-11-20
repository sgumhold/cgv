#pragma once

#include <cgv/base/named.h>
#include <string>

namespace fltk {
	class Widget;
	struct Cursor;
}

#include "lib_begin.h"

/** the fltk base class keeps strings for label and tooltip and 
    implements methods corresponding to the property interface of 
	 cgv::base::base.*/ 
struct CGV_API fltk_base
{
	fltk_base();
	/// store the cursor
	fltk::Cursor* cursor;
	/// store the tooltip as string
	std::string tooltip;
	// store the alignment as string
	std::string alignment;
	// default width and height of the element
	int default_width, default_height;
	// minimal width and height of the element
	int minimum_width, minimum_height;
	/// returns declarations for the reflected properties of a fltk Widget
	std::string get_property_declarations();
	/// set the property of a fltk widget
	bool set_void(fltk::Widget* w, cgv::base::named* nam, const std::string& property, const std::string& type, const void* value);
	/// get the property of a fltk widget 
	bool get_void(const fltk::Widget* w, cgv::base::named* nam, const std::string& property, const std::string& type, void* value);
	/// handle method that ensures that the cursor is shown correctly
	int handle(fltk::Widget* w, int event);
};

/// helper template to support changing the cursor when hovering over a widget
template <class widget>
class CW : public widget
{
public:
	CW() : widget() {}
	CW(const char* l) : widget(l) {}
	CW(int w, int h, const char* l=0) : widget(w,h,l) {}
	CW(int x, int y, int w, int h, const char* l=0) : widget(x,y,w,h,l) {}

	int handle(int event)
	{
		cgv::base::base_ptr bp(static_cast<cgv::base::base*>(this->user_data()));
		fltk_base* fb = bp->get_interface<fltk_base>();
		if (fb) {
			int res = fb->handle(this, event);
			if (res)
				return res;
		}
		return widget::handle(event);
	}
};

/// helper template to support changing the cursor when hovering over a group
template <class group>
class CG : public group
{
public:
	CG(int x, int y, int w, int h, const char* l=0) : group(x,y,w,h,l) {}
	int handle(int event)
	{
		cgv::base::base_ptr bp(static_cast<cgv::base::base*>(this->user_data()));
		fltk_base* fb = bp->get_interface<fltk_base>();
		if (fb) {
			int res = fb->handle(this, event);
			if (res)
				return res;
		}
		return group::handle(event);
	}
};

/// helper template to support changing the cursor when hovering over a group
template <class WI>
class CI : public WI
{
public:
	CI(int w, int h, const char* l=0) : WI(w,h,l) {}
	CI(int x, int y, int w, int h, const char* l=0) : WI(x,y,w,h,l) {}
	int handle(int event)
	{
		cgv::base::base_ptr bp(static_cast<cgv::base::base*>(this->user_data()));
		fltk_base* fb = bp->get_interface<fltk_base>();
		if (fb) {
			int res = fb->handle(this, event);
			if (res)
				return res;
		}
		return WI::handle(event);
	}
};



#include <cgv/config/lib_end.h>
