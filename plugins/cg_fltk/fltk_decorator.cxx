#include "fltk_driver_registry.h"
#include "fltk_base.h"
#include <cgv/type/variant.h>
#include <cgv/utils/scan.h>

#ifdef WIN32
#pragma warning (disable:4311)
#endif
#include <fltk/Divider.h>
#include <fltk/TextDisplay.h>
#ifdef WIN32
#pragma warning (default:4311)
#endif
#include <iostream>

using namespace fltk;
using namespace cgv::type;

struct fltk_heading_decorator : public cgv::base::named, public fltk_base
{
	/// store instance of fltk base functionality class
	int level;
	CW<Widget>* w;

	fltk_heading_decorator(const std::string& name, int x, int y, int _w, int _h) 
		: cgv::base::named(name)
	{
		w = new CW<Widget>(x,y,_w,_h,get_name().c_str());
		w->flags(ALIGN_INSIDE_LEFT);
		w->user_data(static_cast<cgv::base::base*>(this));
		level = 0;
		//w->labelfont(TIMES_BOLD);
		w->labelfont(HELVETICA_BOLD);
		w->labelsize(16);
		w->box(NO_BOX);
	}
	~fltk_heading_decorator()
	{
		delete w;
	}
	std::string get_type_name() const
	{
		return "fltk_line_decorator";
	}
	std::string get_property_declarations()
	{
		return fltk_base::get_property_declarations()+";level:int32";
	}
	/// abstract interface for the setter
	bool set_void(const std::string& property, const std::string& value_type, const void* value_ptr)
	{
		if (fltk_base::set_void(w, this, property, value_type, value_ptr))
			return true;
		if (property == "level") {
			get_variant(level, value_type, value_ptr);
			if (level < 0)
				level = 0;
			if (level > 3)
				level = 3;
			w->labelsize((float)(18-2*level));
		} else {
			return false;
		}
		return true;
	}
	bool get_void(const std::string& property, const std::string& value_type, void* value_ptr)
	{
		if (fltk_base::get_void(w, this, property, value_type, value_ptr))
			return true;
		if (property == "level") {
			set_variant(level, value_type, value_ptr);
			return true;
		}
		return false;
	}
	void* get_user_data() const
	{
		return w;
	}
};

struct fltk_text_decorator : public cgv::base::named, public fltk_base {
	/// store instance of fltk base functionality class
	int level;
	CW<Widget>* w;

	fltk_text_decorator(const std::string& name, int x, int y, int _w, int _h)
		: cgv::base::named(name) {
		w = new CW<Widget>(x, y, _w, _h, get_name().c_str());
		w->flags(ALIGN_INSIDE_TOPLEFT | ALIGN_WRAP);
		w->user_data(static_cast<cgv::base::base*>(this));
		level = 0;
		w->labelfont(HELVETICA);
		w->labelsize(12);
		w->box(NO_BOX);
	}
	~fltk_text_decorator() {
		delete w;
	}
	std::string get_type_name() const {
		return "fltk_text_decorator";
	}
	std::string get_property_declarations() {
		return fltk_base::get_property_declarations() + ";level:int32";
	}
	/// abstract interface for the setter
	bool set_void(const std::string& property, const std::string& value_type, const void* value_ptr) {
		
		return fltk_base::set_void(w, this, property, value_type, value_ptr);
	}
	bool get_void(const std::string& property, const std::string& value_type, void* value_ptr) {
		
		return fltk_base::get_void(w, this, property, value_type, value_ptr);
	}
	void* get_user_data() const {
		return w;
	}
};

struct fltk_separator_decorator : public cgv::base::named, public fltk_base
{
	/// store instance of fltk base functionality class
	CW<Divider>* d;

	fltk_separator_decorator(const std::string& name, int x, int y, int _w, int _h) 
		: cgv::base::named(name)
	{
		d = new CW<Divider>();
		d->resize(x,y,_w,_h);
		d->user_data(static_cast<cgv::base::base*>(this));
	}
	~fltk_separator_decorator()
	{
		delete d;
	}
	std::string get_type_name() const
	{
		return "fltk_separator_decorator";
	}
	std::string get_property_declarations()
	{
		return fltk_base::get_property_declarations();
	}
	/// abstract interface for the setter
	bool set_void(const std::string& property, const std::string& value_type, const void* value_ptr)
	{
		return fltk_base::set_void(d, this, property, value_type, value_ptr);
	}
	bool get_void(const std::string& property, const std::string& value_type, void* value_ptr)
	{
		return fltk_base::get_void(d, this, property, value_type, value_ptr);
	}
	void* get_user_data() const
	{
		return (Widget*)d;
	}
};



struct decorator_factory : public abst_decorator_factory
{
	base_ptr create(const std::string& label, 
			const std::string& gui_type, int x, int y, int w, int h)
	{
		if (gui_type == "heading")
			return base_ptr(new fltk_heading_decorator(label,x,y,w,h));
		if (gui_type == "text")
			return base_ptr(new fltk_text_decorator(label, x, y, w, h));
		if (gui_type == "separator")
			return base_ptr(new fltk_separator_decorator(label,x,y,w,h));
		return base_ptr();
	}
};

decorator_factory_registration<decorator_factory> decorator_fac_reg;
