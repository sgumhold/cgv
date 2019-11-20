#include "fltk_base.h"
#include "fltk_event.h"
#include <iostream>
#include <cgv/gui/shortcut.h>
#include <cgv/type/variant.h>
#include <cgv/media/image/image_reader.h>
#include <cgv/media/image/image_writer.h>
#include <cgv/utils/convert.h>
#include <cgv/utils/scan.h>
#include <cgv/data/data_view.h>

#ifdef WIN32
#pragma warning(disable:4311)
#endif
#include <fltk/Widget.h>
#include <fltk/Group.h>
#include <fltk/Cursor.h>
#include <fltk/events.h>
#include <fltk/SharedImage.h>
#ifdef WIN32
#pragma warning(default:4311)
#endif

using namespace cgv::media::image;
using namespace cgv::data;
using namespace cgv::type;
using namespace fltk;


fltk_base::fltk_base()
{
	cursor = fltk::CURSOR_DEFAULT;
	default_width = 0;
	default_height = 0;
	minimum_width = 0;
	minimum_height = 0;
}

std::string fltk_base::get_property_declarations()
{
	return  "x:int32;y:int32;w:int32;h:int32;mw:int32;mh:int32;dw:int32;dh:int32;"
			"image:string;fit_image:bool;label:string;name:string;tooltip:string;"
			"active:bool;show:bool;color:int32;text_color:int32;label_color:int32;align:string;dolayout:int32;cursor:string";
}

class cgvImage : public SharedImage 
{
	cgvImage() { }
	static SharedImage* create() 
	{
		return new cgvImage; 
	}
public:
	static bool test(const uchar* datas, unsigned size=0);
	static SharedImage* get(const char* name, const uchar* datas = 0) {
		return SharedImage::get(create, name, datas);
	}
	bool fetch()
	{
		data_view dv;
		data_format df;
		image_reader r(df);
		if (!r.read_image(name, dv)) {
			std::cerr << "WARNING: could not read image " << name << "! Maybe cmi_io-plugin is necessary." << std::endl;
			return false;
		}
		PixelType pt = RGB;
		switch (df.get_standard_component_format()) {
		case CF_L : pt = MONO; break;
		case CF_RGB : pt = RGB; break;
		case CF_RGBA : pt = RGBA; break;
		default:
			std::cerr << "could not deduce fltk::PixelType of image " << name << std::endl;
			return false;
		}
		setpixeltype(pt);
		int h = df.get_height();
		setsize(df.get_width(), h);
		for (int i=0; i<h; ++i) 
			setpixels(dv.get_ptr<unsigned char>()+dv.get_step_size(0)*i, i);
		return true;
	}
};


/// abstract interface for the setter
bool fltk_base::set_void(fltk::Widget* w, cgv::base::named* nam, const std::string& property, const std::string& value_type, const void* value_ptr)
{
	if (property == "x")
		w->position(variant<int32_type>::get(value_type, value_ptr),w->y());
	else if (property == "y")
		w->position(w->x(),variant<int32_type>::get(value_type, value_ptr));
	else if (property == "w") {
		w->resize(variant<int32_type>::get(value_type, value_ptr),w->h());
		//w->layout();
	}
	else if (property == "h") {
		w->resize(w->w(),variant<int32_type>::get(value_type, value_ptr));
		//w->layout();
	}
	else if (property == "label")
		w->copy_label(variant<std::string>::get(value_type, value_ptr).c_str());
	else if (property == "name") 
		nam->set_name(variant<std::string>::get(value_type, value_ptr));
	else if (property == "image") 
		w->image(cgvImage::get(variant<std::string>::get(value_type, value_ptr).c_str()));
	else if (property == "fit_image") {
		if (variant<bool>::get(value_type, value_ptr))
			w->flags(w->flags()|RESIZE_FIT);
		else
			w->flags(w->flags()&~RESIZE_FIT);
	}
	else if (property == "tooltip") {
		tooltip = variant<std::string>::get(value_type, value_ptr);
		w->tooltip(tooltip.c_str());
	}
	else if (property == "active")
		w->activate(variant<bool>::get(value_type, value_ptr));
	else if (property == "color")
		w->color(256*variant<int32_type>::get(value_type, value_ptr));
	else if (property == "text_color")
		w->textcolor(256*variant<int32_type>::get(value_type, value_ptr));
	else if (property == "label_color")
		w->labelcolor(256*variant<int32_type>::get(value_type, value_ptr));
	else if (property == "show") {
		if (variant<bool>::get(value_type, value_ptr))
			w->show();
		else
			w->hide();
	}
	else if (property == "align") {
		std::string align_info;
		get_variant(align_info,value_type,value_ptr);
		unsigned int flags = w->flags() & ~fltk::ALIGN_MASK;
		for (unsigned int i=0; i<align_info.length(); ++i)
			switch(align_info[i]) {
				case 'T' :
					flags |= fltk::ALIGN_TOP;
					break;
				case 't' :
					flags |= fltk::ALIGN_INSIDE_TOP;
					break;
				case 'B' :
					flags |= fltk::ALIGN_BOTTOM;
					break;
				case 'b' :
					flags |= fltk::ALIGN_INSIDE_BOTTOM;
					break;
				case 'L' :
					flags |= fltk::ALIGN_LEFT;
					break;
				case 'l' :
					flags |= fltk::ALIGN_INSIDE_LEFT;
					break;
				case 'R' :
					flags |= fltk::ALIGN_RIGHT;
					break;
				case 'r' :
					flags |= fltk::ALIGN_INSIDE_RIGHT;
					break;
			}
		w->flags(flags);
	}
	else if (property == "shortcut") {
		cgv::gui::shortcut sc;
		if (value_type == cgv::type::info::type_name<cgv::gui::shortcut>::get_name())
			sc = * ( (cgv::gui::shortcut*) value_ptr);
		else {
			std::string sc_str;
			get_variant(sc_str, value_type, value_ptr);
			if (!cgv::utils::from_string(sc, sc_str)) {
				std::cerr << "error: could not parse shortcut from string: " << sc_str << std::endl;
				return false;
			}
		}
		w->shortcut(fltk_shortcut(sc));
	}
	else if (property == "alignment") 
		get_variant(alignment, value_type, value_ptr);
	else if (property == "dw")
		get_variant(default_width, value_type, value_ptr);
	else if (property == "dh")
		get_variant(default_height, value_type, value_ptr);	
	else if (property == "mw")
		get_variant(minimum_width, value_type, value_ptr);
	else if (property == "mh")
		get_variant(minimum_height, value_type, value_ptr);
	else if (property == "dolayout")
		w->layout();
	else if (property == "cursor") {
		std::string value;
		get_variant(value, value_type, value_ptr);
		int ei = cgv::utils::get_element_index(value,"default,arrow,cross,wait,insert,hand,help,move,ns,we,nwse,nesw,no,invisible",',');
		if (ei == -1)
			ei = 0;
		fltk::Cursor* cursors[] = {
			fltk::CURSOR_DEFAULT,
			fltk::CURSOR_ARROW,
			fltk::CURSOR_CROSS,
			fltk::CURSOR_WAIT,
			fltk::CURSOR_INSERT,
			fltk::CURSOR_HAND,
			fltk::CURSOR_HELP,
			fltk::CURSOR_MOVE,
			fltk::CURSOR_NS,
			fltk::CURSOR_WE,
			fltk::CURSOR_NWSE,
			fltk::CURSOR_NESW,
			fltk::CURSOR_NO,
			fltk::CURSOR_NONE
		};
		cursor = cursors[ei];
		w->cursor(cursor);
	}
	else
		return false;
	w->redraw();
	if (w->parent())
		w->parent()->redraw();
	return true;
}

/// abstract interface for the getter
bool fltk_base::get_void(const fltk::Widget* w, cgv::base::named* nam, const std::string& property, const std::string& value_type, void* value_ptr)
{
	if (property == "x")
		set_variant(w->x(), value_type, value_ptr);
	else if (property == "y")
		set_variant(w->y(), value_type, value_ptr);
	else if (property == "w")
		set_variant(w->w(), value_type, value_ptr);
	else if (property == "h")
		set_variant(w->h(), value_type, value_ptr);
	else if (property == "label")
		set_variant(w->label(), value_type, value_ptr);
	else if (property == "name")
		set_variant(nam->get_name(), value_type, value_ptr);
	else if (property == "image") 
		set_variant(w->image()?w->image()->name():"", value_type, value_ptr);
	else if (property == "fit_image")
		set_variant((w->flags()&RESIZE_FIT)==RESIZE_FIT, value_type, value_ptr);
	else if (property == "tooltip")
		set_variant(w->tooltip(), value_type, value_ptr);
	else if (property == "active")
		set_variant(w->active(), value_type, value_ptr);
	else if (property == "color")
		set_variant((int)w->color()/256, value_type, value_ptr);
	else if (property == "text_color")
		set_variant((int)w->textcolor()/256, value_type, value_ptr);
	else if (property == "label_color")
		set_variant((int)w->labelcolor()/256, value_type, value_ptr);
	else if (property == "show") {
		set_variant(w->visible(), value_type, value_ptr);
	}
	else if (property == "alignment") 
		set_variant(alignment, value_type, value_ptr);
	else if (property == "dw")
		set_variant(default_width, value_type, value_ptr);
	else if (property == "dh")
		set_variant(default_height, value_type, value_ptr);
	else if (property == "mw")
		set_variant(minimum_width, value_type, value_ptr);
	else if (property == "mh")
		set_variant(minimum_height, value_type, value_ptr);
	else
		return false;
	return true;
}

/// handle method that ensures that the cursor is shown correctly
int fltk_base::handle(fltk::Widget* w, int event)
{
	if (event == fltk::ENTER || event == fltk::MOVE)
		w->cursor(cursor);
	return 0;
}

