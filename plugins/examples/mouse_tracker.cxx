#include "mouse_tracker.h"
#include <cgv/base/find_action.h>
#include <cgv/gui/key_event.h>
#include <cgv/gui/mouse_event.h>
#include <sstream>

using namespace cgv::gui;
using namespace cgv::render;

/// construct from name which is necessary construction argument to node
mouse_tracker::mouse_tracker(const char* name) : node(name), picked_point(0, 0, 0)
{
	font_size = 20;
	bold = false;
	italic = false;
	font_idx = 0;
	view_ptr = 0;
	x = y = 0;	
	have_picked_point = false;
	cgv::media::font::enumerate_font_names(font_names);
}

/// find view ptr
bool mouse_tracker::init(cgv::render::context& ctx)
{
	view_ptr = find_view_as_node();
	return view_ptr != 0;
}

/// show internal values
void mouse_tracker::stream_stats(std::ostream& os)
{
	os << "mouse tracker: " << font_names[font_idx]; 
	if (bold)
		os << "+bold";
	if (italic)
		os << "+italic";
	os << ":" << font_size << std::endl;
}

/// necessary method of event_handler
bool mouse_tracker::handle(event& e)
{
	if (e.get_kind() == EID_KEY) {
		key_event& ke = static_cast<key_event&>(e);
		if (ke.get_action() == KA_PRESS) {
			switch (ke.get_key()) {
			case 'B' : 
				bold = !bold;
				ff.clear();
				post_redraw();
				return true;
			case 'I' : 
				italic = !italic;
				ff.clear();
				post_redraw();
				return true;
			case KEY_Up :
				font_idx += 1;
				if (font_idx >= font_names.size())
					font_idx = 0;
				ff.clear();
				post_redraw();
				return true;
			case KEY_Down :
				if (font_idx == 0 && !font_names.size() == 0)
					font_idx = (int)font_names.size()-1;
				else
					--font_idx;
				ff.clear();
				post_redraw();
				return true;
			case KEY_Page_Up :
				font_size += 1;
				post_redraw();
				return true;
			case KEY_Page_Down :
				if (font_size > 4) {
					font_size -= 1;
					post_redraw();
				}
				return true;
			}
		}
	}
	else if (e.get_kind() == EID_MOUSE) {
		mouse_event& me = static_cast<mouse_event&>(e);
		switch (me.get_action()) {
		case MA_ENTER : 
			static_cast<drawable*>(this)->set_active(true);
			post_redraw();
			return true;
		case MA_LEAVE :
			static_cast<drawable*>(this)->set_active(false);
			post_redraw();
			return true;
		case MA_MOVE :
			x = me.get_x();
			y = me.get_y();
			have_picked_point = false;
			if (view_ptr)
				have_picked_point = get_world_location(x, y, *view_ptr, picked_point);
			post_redraw();
			return false;
		case MA_DRAG :
			x = me.get_x();
			y = me.get_y();
			have_picked_point = false;
			if (view_ptr)
				have_picked_point = get_world_location(x, y, *view_ptr, picked_point);
			post_redraw();
			return false;
		default:
			return false;
		}
	}
	return false;
}

/// necessary method of event_handler
void mouse_tracker::stream_help(std::ostream& os)
{
	os << "Mouse Tracker:\a\n"
	      << "change font: <Down,Up>\n"
	      << "change font size: <PgDn,PgUp>\n"
		  << "toggle bold<B> / italic<I>\n\b";
}

#include <cgv_gl/gl/gl.h>
#include <cgv/render/shader_program.h>

/// optional method of drawable
void mouse_tracker::draw(context& ctx)
{
	if (ff.empty()) {
		cgv::media::font::font_ptr f = cgv::media::font::find_font(font_names[font_idx]);
		ff = f->get_font_face((bold?cgv::media::font::FFA_BOLD:0)+
			                  (italic?cgv::media::font::FFA_ITALIC:0));
	}
	if (ff.empty())
		return;
//	ctx.ref_default_shader_program().enable(ctx);
		ctx.set_color(rgb(1.0f, 0.0f, 1.0f));
		ctx.push_pixel_coords();
			ctx.enable_font_face(ff, font_size);
			std::stringstream ss;
			ss << "(" << x << "," << y << ")";
			if (have_picked_point)
				ss << "->[" << picked_point << "]";
			std::string s = ss.str();
			float f = (float)x/ctx.get_width();
			ctx.set_cursor(x-(int)(f*ff->measure_text_width(s,font_size)),y-4);
			ctx.output_stream() << s.c_str();
			ctx.output_stream().flush();
		ctx.pop_pixel_coords();
//	ctx.ref_default_shader_program().disable(ctx);
}

#include <cgv/base/register.h>

/// register a factory to create new cubes
cgv::base::factory_registration_1<mouse_tracker,const char*> tracker_fac("new/events/mouse tracker", 'M', "mouse tracker", true);

