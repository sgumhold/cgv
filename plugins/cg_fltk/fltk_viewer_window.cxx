#include "fltk_viewer_window.h"
#include "fltk_driver.h"
#include "fltk_event.h"
#include <cgv/gui/provider.h>
#include <cgv/gui/theme_info.h>
#include <cgv/signal/rebind.h>
#include <cgv/base/base_generator.h>
#include <cgv/base/register.h>
#include <cgv/utils/scan.h>
#include <cgv/utils/tokenizer.h>
#include <cgv/type/variant.h>
#include <cgv/render/drawable.h>
#include <cgv/os/resources.h>

#ifdef WIN32
#pragma warning (disable:4311)
#endif
#include <fltk/Item.h>
#include <fltk/Cursor.h>
#include <fltk/ItemGroup.h>
#include <fltk/TiledGroup.h>
#include <fltk/TabGroup.h>
#include <fltk/MenuBuild.h>
#include <fltk/PackedGroup.h>
#include <fltk/Button.h>
#include <fltk/run.h>
#ifdef WIN32
#include <fltk/win32.h>
#include <windows.h>
#endif
#include <fltk/Monitor.h>
#include <fltk/events.h>
#ifdef WIN32
#pragma warning (default:4311)
#endif

#include <iostream>
#include <math.h>

using namespace cgv::base;
using namespace cgv::gui;
using namespace cgv::utils;

void destroy_callback(fltk::Widget* w)
{
	if(!cgv::base::request_exit_from_all_objects())
		return;

	fltk_viewer_window* v = static_cast<fltk_viewer_window*>(w);
	if (!v->get_view().empty())
		v->get_view()->destroy();
	v->destroy();

	window_ptr wp(v);

	cgv::base::unregister_object(wp, "");

	fltk_driver* d = cgv::gui::get_gui_driver()->get_interface<fltk_driver>();
	if (d) {
		d->remove_window(wp);
	}
	else {
		std::cerr << "could not notify driver!!" << std::endl;
	}
	(void*&)wp = 0;
}

void fltk_viewer_window::on_tab_group_selection_change(base_ptr bp, bool selected)
{
	if (!selected)
		return;
	for (unsigned ci=0; ci<view->get_nr_children(); ++ci) {
		provider* p = view->get_child(ci)->get_interface<provider>();
		if (p) {
			if (get_provider_parent(p)->cast<base>() == bp) {
				view->get_interface<event_handler>()->set_focused_child((int)ci);
				break;
			}
		}
	}
}


/// construct application
fltk_viewer_window::fltk_viewer_window(int w, int h, const std::string& _title)
	: cgv::gui::window("Main"), title(_title), 
	  fltk::Window(w,h,"")

{
	fltk::Window::label(title.c_str());

	window_state = WS_REGULAR;
	fullscreen_monitors = MS_MONITOR_CURRENT;
	menu_visible = true;
	gui_visible = true;
	theme_name = "light";
	menu = 0;
	callback(destroy_callback);

	menu_height = 24;
	menu_right = true;

	begin();
		main_group = new DockableGroup(0, 0, w, h, "");
		main_group->spacing(1);
		main_group->begin();
			menu = new fltk::MenuBar(0, 0, w, menu_height);
			view = fltk_gl_view_ptr(new fltk_gl_view(0, 0, w, h, "GL View"));
			if(menu_right)
				tab_group = fltk_tab_group_ptr(new fltk_tab_group((int)(2.85*w/4), 0, (int)(1.15*w/4), h, ""));
			else
				tab_group = fltk_tab_group_ptr(new fltk_tab_group(0, 21, (int)(1.15*w/4), h-menu_height, ""));

			connect(tab_group->on_selection_change, this, &fltk_viewer_window::on_tab_group_selection_change);
			//connect(view->on_remove_child, this, &fltk_viewer_window::on_remove_child);
		main_group->end();
		main_group->resizable(view->get_interface<fltk::Widget>());
		ensure_dock_state();
	end();
	resizable(main_group);	
	update_member(&menu_visible);
	update_member(&gui_visible);
	append_child(view);
	append_child(tab_group);
}

void fltk_viewer_window::on_register()
{
	tab_group->register_object(base_ptr(this),"");
	tab_group->register_object(view, "");
}

/// show the window. This needs to be called after creation to make the window visible
void fltk_viewer_window::show(bool modal)
{
	if (modal)
		fltk::Window::exec();
	else
		fltk::Window::show();

	set_theme();
}

/// hide the window
void fltk_viewer_window::hide()
{
	fltk::Window::hide();
}

/// return a shortcut to activate the gui without menu navigation
cgv::gui::shortcut fltk_viewer_window::get_shortcut() const
{
	return cgv::gui::shortcut('M', EM_CTRL);
}

/// return a path in the main menu to select the gui
std::string fltk_viewer_window::get_menu_path() const
{
	return "Menu/Main";
}

void fltk_viewer_window::menu_change_cb()
{
	menu_visible = !menu_visible;
	if (menu_shown())
		hide_menu(false);
	else
		show_menu(false);
}

void fltk_viewer_window::gui_change_cb()
{
	gui_visible = !gui_visible;
	if (gui_shown())
		hide_gui(false);
	else
		show_gui(false);
}

void fltk_viewer_window::theme_change_cb() {
	int idx = static_cast<int>(theme_idx) - 1;

	switch(idx) {
	case -1: theme_name = "legacy"; break;
	case 0: theme_name = "light"; break;
	case 1: theme_name = "mid"; break;
	case 2: theme_name = "dark"; break;
	case 3: theme_name = "darkest"; break;
	default:
	{
		theme_name = "light";
		theme_idx = static_cast<cgv::type::DummyEnum>(0);
		update_member(&theme_idx);
		idx = 0;
	} break;
	}

	fltk::theme_idx_ = idx;
	fltk::reload_theme();

	{ // TODO: maybe move this to some other place
		auto& theme = cgv::gui::theme_info::instance();

		// to change menu position:
		//menu_right = false;
		//ensure_dock_state();

		if(fltk::theme_idx_ < 0)
			main_group->spacing(3);
		else
			main_group->spacing(1);

		theme.spacing(main_group->spacing());

		uchar r, g, b;
		fltk::split_color(fltk::get_theme_color(fltk::THEME_BACKGROUND_COLOR), r, g, b);
		theme.background(r, g, b);
		fltk::split_color(fltk::get_theme_color(fltk::THEME_GROUP_COLOR), r, g, b);
		theme.group(r, g, b);
		fltk::split_color(fltk::get_theme_color(fltk::THEME_CONTROL_COLOR), r, g, b);
		theme.control(r, g, b);
		fltk::split_color(fltk::get_theme_color(fltk::THEME_BORDER_COLOR), r, g, b);
		theme.border(r, g, b);
		fltk::split_color(fltk::get_theme_color(fltk::THEME_TEXT_COLOR), r, g, b);
		theme.text(r, g, b);
		fltk::split_color(fltk::get_theme_color(fltk::THEME_TEXT_BACKGROUND_COLOR), r, g, b);
		theme.text_background(r, g, b);
		fltk::split_color(fltk::get_theme_color(fltk::THEME_SELECTION_COLOR), r, g, b);
		theme.selection(r, g, b);
		fltk::split_color(fltk::get_theme_color(fltk::THEME_HIGHLIGHT_COLOR), r, g, b);
		theme.highlight(r, g, b);
		fltk::split_color(fltk::get_theme_color(fltk::THEME_WARNING_COLOR), r, g, b);
		theme.warning(r, g, b);
		fltk::split_color(fltk::get_theme_color(fltk::THEME_SHADOW_COLOR), r, g, b);
		theme.shadow(r, g, b);
		// set theme index only after all colors have been updated
		theme.set_index(idx);
	}

	if(tab_group) {
		tab_group->update();
		post_recreate_gui();
	}
}

void fltk_viewer_window::set_theme() {
	int idx = 0;
	bool found = false;
	if(theme_name == "legacy") {
		idx = -1;
		found = true;
	} else if(theme_name == "light") {
		idx = 0;
		found = true;
	} else if(theme_name == "mid") {
		idx = 1;
		found = true;
	} else if(theme_name == "dark") {
		idx = 2;
		found = true;
	} else if(theme_name == "darkest") {
		idx = 3;
		found = true;
	}

	if(!found) {
		idx = 0;
		theme_name = "light";
	}

	theme_idx = static_cast<cgv::type::DummyEnum>(idx + 1);
	update_member(&theme_idx);
	theme_change_cb();
}

bool fltk_viewer_window::ws_change_cb(control<WindowState>& c)
{
	set_window_state(c.get_new_value(),fullscreen_monitors,false);
	return true;
}

bool fltk_viewer_window::ms_change_cb(control<MonitorSelection>& c)
{
	set_window_state(window_state,c.get_new_value(),false);
	return true;
}

/// you must overload this for gui creation
void fltk_viewer_window::create_gui()
{
	provider::add_decorator("Main Settings", "heading");
	connect_copy(provider::add_control("Menu", menu_visible, "check")->value_change,
		rebind(this, &fltk_viewer_window::menu_change_cb));
	connect_copy(provider::add_control("Gui", gui_visible, "check")->value_change,
		rebind(this, &fltk_viewer_window::gui_change_cb));
	connect_copy(provider::add_control("Theme", theme_idx, "dropdown", "enums='Legacy,Light,Mid,Dark,Darkest'")->value_change,
		rebind(this, &fltk_viewer_window::theme_change_cb));
	connect(provider::add_control("State", window_state, "dropdown", "enums='regular;minimized;maximized;fullscreen'")->check_value,
		this, &fltk_viewer_window::ws_change_cb);
	connect(provider::add_control("Fullscreen Monitors", fullscreen_monitors, "dropdown", "enums='current;1;2;1+2;3;1+3;2+3;1+2+3'")->check_value,
		this, &fltk_viewer_window::ms_change_cb);
}

/// returns fltk_viewer_window
std::string fltk_viewer_window::get_type_name() const
{
	return "fltk_viewer_window";
}
/// passes update over to the fltk_gui_group if it exists
void fltk_viewer_window::update()
{
//	static_cast<fltk::Window*>(this)->handle(fltk::KEY);
	if (!tab_group.empty())
		tab_group->update();
	redraw();
}

void fltk_viewer_window::on_set(void* member_ptr) {
	if(member_ptr == &theme_name) {
		set_theme();
	}
}

bool fltk_viewer_window::self_reflect(cgv::reflect::reflection_handler& rh) {
	return
		rh.reflect_member("theme", theme_name);
}

/// returns the property declaration
std::string fltk_viewer_window::get_property_declarations()
{
	std::string props = fltk_base::get_property_declarations();
	if (view)
		props += std::string(";")+view->get_property_declarations()+";"+tab_group->get_property_declarations();
	return props+";bg_clr_idx:uint32;gui:bool;icon:int32;menu:bool;menu_order:string;dock_order:string;state:string;title:string;W:uint32;H:uint32";
}

void fltk_viewer_window::ensure_menu_order()
{
	if (menu->empty() || menu_order.empty())
		return;
	for (int i=0; i<menu->children()-1; ++i) {
		unsigned int pi = (unsigned int) cgv::utils::get_element_index(menu->child(i)->label()?menu->child(i)->label():"",menu_order);
		for (int j=i+1; j<menu->children(); ++j) {
			unsigned int pj = (unsigned int) cgv::utils::get_element_index(menu->child(j)->label()?menu->child(j)->label():"",menu_order);
			if (pj < pi)
				menu->swap(i,j);
		}
	}
}

unsigned int fltk_viewer_window::get_dock_idx(int i) const
{
	fltk::Widget* w = main_group->dock_order[i];
	std::string res;
	if (w == menu)
		res = "menu";
	else {
		void *ud = w->user_data();
		base* bp = static_cast<base*>(ud);
		named_ptr np = bp->get_named();
		if (!np)
			res = w->label();
		else
			res = np->get_name();
	}
	return (unsigned int) cgv::utils::get_element_index(res, dock_order);
}

void fltk_viewer_window::ensure_dock_order()
{
	if (main_group->empty() || dock_order.empty())
		return;
	for (int i=0; i<(int)main_group->dock_order.size()-1; ++i) {
		unsigned int pi = get_dock_idx(i);
		for (int j=i+1; j<(int)main_group->dock_order.size(); ++j) {
			unsigned int pj = get_dock_idx(j);
			if (pj < pi)
				std::swap(main_group->dock_order[i],main_group->dock_order[j]);
		}
	}
}

void fltk_viewer_window::ensure_dock_state() {
	if(menu_visible && menu)
		main_group->undock(menu);
	if(gui_visible)
		main_group->undock(static_cast<fltk::Widget*>(tab_group->get_user_data()));

	if(menu_right) {
		if(gui_visible) main_group->dock(static_cast<fltk::Widget*>(tab_group->get_user_data()), 0, true);
		if(menu_visible && menu) main_group->dock(menu, 1, false);
	} else {
		if(menu_visible && menu) main_group->dock(menu, 1, false);
		if(gui_visible) main_group->dock(static_cast<fltk::Widget*>(tab_group->get_user_data()), 2, true);
	}
}

/// abstract interface for the setter implemented via the fltk_gui_group
bool fltk_viewer_window::set_void(const std::string& property, const std::string& value_type, const void* value_ptr)
{
	if (property == "menu_order") {
		cgv::type::get_variant(menu_order,value_type,value_ptr);
		ensure_menu_order();
		return true;
	}
	if (property == "dock_order") {
		cgv::type::get_variant(dock_order,value_type,value_ptr);
		ensure_dock_order();
		return true;
	}
	if (property == "W") {
		if (view) {
			unsigned int new_w; 
			cgv::type::get_variant(new_w, value_type, value_ptr);
			resize(w() + new_w - view->w(), h());

			node_ptr(view)->set_parent(node_ptr());
			view->resize(new_w, view->h());
			node_ptr(view)->set_parent(node_ptr(this));
			return true;
		}
		return false;
	}
	if (property == "H") {
		if (view) {
			unsigned int new_h; cgv::type::get_variant(new_h, value_type, value_ptr);
			resize(w(), h() + new_h - view->h());
			node_ptr(view)->set_parent(node_ptr());
			view->resize(view->w(), new_h);
			node_ptr(view)->set_parent(node_ptr(this));
			return true;
		}
		return false;
	}
	if (property == "title") {
		cgv::type::get_variant(title, value_type, value_ptr);
		label(title.c_str());
		return true;
	}
	if (property == "icon") {
		int i;
		cgv::type::get_variant(i, value_type, value_ptr);
#ifdef WIN32
#	ifdef _UNICODE
		HINSTANCE hi = GetModuleHandle(cgv::utils::str2wstr(cgv::base::ref_prog_name()).c_str());
#	else
		HMODULE hi = GetModuleHandle(cgv::base::ref_prog_name().c_str());
#	endif
		auto ic = LoadIcon(hi, MAKEINTRESOURCE(i));
		icon(ic);
		return true;
#else
		return false;
#endif
	}
	if (fltk_base::set_void(this, this, property, value_type, value_ptr)) {
		if (property == "cursor" && view)
			view->set_void(property, value_type, value_ptr);
		if (tab_group.empty() || property == "x" || property == "y" || property == "w" || property == "h")
			return true;
		return tab_group->set_void(property, value_type, value_ptr);
	}
	if (view && view->set_void(property, value_type, value_ptr))
		return true;
	if (tab_group && tab_group->set_void(property, value_type, value_ptr))
		return true;

	if (property == "status_info") {
		if (cgv::type::variant<bool>::get(value_type, value_ptr))
			show_gui();
		else
			hide_gui();
	}
	else if (property == "gui") {
		if (cgv::type::variant<bool>::get(value_type, value_ptr))
			show_gui();
		else
			hide_gui();
	}
	else if (property == "menu") {
		if (cgv::type::variant<bool>::get(value_type, value_ptr))
			show_menu();
		else
			hide_menu();
	}
	else if (property == "state") {
		std::string s = cgv::type::variant<std::string>::get(value_type, value_ptr);
		WindowState ws = WS_REGULAR;
		MonitorSelection ms = MS_MONITOR_CURRENT;
		if (s == "minimized")
			ws = WS_MINIMIZED;
		else if (s == "maximized")
			ws = WS_MAXIMIZED;
		else if (s.substr(0,10) == "fullscreen") {
			ws = WS_FULLSCREEN;
			if (s.length() > 10) {
				if (s[10] == '(' && s[s.length()-1] == ')') {
					for (unsigned int i=0; i < s.size()-12; i+=2) {
						int mi = s[11+i]-'0';
						switch (mi) {
						case 1 : ms = MonitorSelection(ms+MS_MONITOR_1); break;
						case 2: ms = MonitorSelection(ms + MS_MONITOR_2); break;
						case 3: ms = MonitorSelection(ms + MS_MONITOR_3); break;
						case 4: ms = MonitorSelection(ms + MS_MONITOR_4); break;
						}
					}
				}
			}
		}
		set_window_state(ws, ms);
	}
	else
		return base::set_void(property, value_type, value_ptr);
	return true;
}

/// dispatch a cgv event to the gl view
bool fltk_viewer_window::dispatch_event(cgv::gui::event& e)
{
	return get_view()->dispatch_event(e);
}

/// overload fltk handle method
int fltk_viewer_window::handle(int event)
{
	if (event == fltk::KEY) {
		unsigned char modifiers = 0;
		if (fltk::event_state() & fltk::SHIFT) 
			modifiers += EM_SHIFT;
		if (fltk::event_state() & fltk::ALT) 
			modifiers += EM_ALT;
		if (fltk::event_state() & fltk::CTRL) 
			modifiers += EM_CTRL;
		if (fltk::event_state() & fltk::META) 
			modifiers += EM_META;
		unsigned short k = fltk::event_key();
		switch (fltk::event_key()) {
		case fltk::F8Key :
			if (modifiers == EM_SHIFT) {
				if (gui_shown())
					hide_gui();
				else
					show_gui();
				return 1;
			}
			if (modifiers == EM_ALT) {
				if (menu_shown())
					hide_menu();
				else
					show_menu();
				return 1;
			}
			break;
		case fltk::MenuKey :
			if (modifiers == EM_SHIFT) {
				if (gui_shown())
					hide_gui();
				else
					show_gui();
			}
			else {
				if (menu_shown())
					hide_menu();
				else
					show_menu();
			}
			return 1;
		case fltk::F11Key :
			if (get_window_state() == WS_FULLSCREEN) {
				if (modifiers == EM_SHIFT) {
					MonitorSelection ms = MonitorSelection((fullscreen_monitors+1)&3);
					set_window_state(get_window_state(), ms);
				}
				else if (modifiers == 0) {
					set_window_state(WS_REGULAR,fullscreen_monitors);
					show_menu();
					show_gui();
					return 1;
				}
			}
			else if (modifiers == 0) {
				hide_menu();
				hide_gui();
				set_window_state(WS_FULLSCREEN,fullscreen_monitors);
				return 1;
			}
			break;
		}
	}

	return fltk::Window::handle(event);
}


/// abstract interface for the getter implemented via the fltk_gui_group
bool fltk_viewer_window::get_void(const std::string& property, const std::string& value_type, void* value_ptr)
{
	if (property == "menu_order") {
		cgv::type::set_variant(menu_order, value_type, value_ptr);
		return true;
	}
	if (property == "dock_order") {
		cgv::type::set_variant(dock_order, value_type, value_ptr);
		return true;
	}
	if (property == "W") {
		if (view) {
			cgv::type::set_variant(view->w(), value_type, value_ptr);
			return true;
		}
		return false;
	}
	if (property == "H") {
		if (view) {
			cgv::type::set_variant(view->h(), value_type, value_ptr);
			return true;
		}
		return false;
	}
	if (property == "title") {
		cgv::type::set_variant(title, value_type, value_ptr);
		return true;
	}
	if (fltk_base::get_void(this, this, property, value_type, value_ptr))
		return true;

	if (view && view->get_void(property, value_type, value_ptr))
		return true;
	if (tab_group && tab_group->get_void(property, value_type, value_ptr))
		return true;

	if (property == "gui")
		cgv::type::set_variant(gui_shown(), value_type, value_ptr);
	else if (property == "menu")
		cgv::type::set_variant(menu_shown(), value_type, value_ptr);
	else if (property == "bg_clr_idx")
		cgv::type::set_variant(view ? view->get_bg_clr_idx() : 0, value_type, value_ptr);
	else if (property == "state") {
		std::string s;
		WindowState ws = get_window_state();
		switch (ws) {
		case WS_REGULAR : s = "regular"; break;
		case WS_MINIMIZED : s = "minimized"; break;
		case WS_MAXIMIZED : s = "maximized"; break;
		default:
			s = "fullscreen";
			if (ws > WS_FULLSCREEN) {
				s += '(';
				if (fullscreen_monitors & MS_MONITOR_1) s += '1';
				if (fullscreen_monitors & MS_MONITOR_2) s += '2';
				s += ')';
			}
		}
		cgv::type::set_variant(s, value_type, value_ptr);
	}
	else
		return base::get_void(property, value_type, value_ptr);
	return true;
}

/// returns the result of get_user_data of the fltk_gui_group
void* fltk_viewer_window::get_user_data() const
{
	return const_cast<fltk::Widget*>(static_cast<const fltk::Widget*>(static_cast<const fltk::Window*>(this)));
}

/// overload and use the fltk_gui_group implementation
unsigned int fltk_viewer_window::remove_child(base_ptr child)
{
	return view->remove_child(child);
}

/// overload and use the fltk_gui_group implementation
void fltk_viewer_window::remove_all_children()
{
	unsigned int n = get_nr_children();
	for (unsigned int i=n; i>0;)
		remove_child(get_child(--i));
}

void fltk_viewer_window::set_fullscreen(MonitorSelection ms)
{
	const fltk::Monitor* mons;
	int n = fltk::Monitor::list(&mons);
	cgv::media::axis_aligned_box<int, 2> rect;
	if ((ms & MS_MONITOR_1) != 0)
		rect.add_axis_aligned_box(cgv::media::axis_aligned_box<int, 2>(
			cgv::math::fvec<int, 2>(mons[0].work.x(), mons[0].work.y()),
			cgv::math::fvec<int, 2>(mons[0].work.x() + mons[0].work.w(), mons[0].work.y() + mons[0].work.h())));
	if ((ms & MS_MONITOR_2) != 0)
		rect.add_axis_aligned_box(cgv::media::axis_aligned_box<int, 2>(
			cgv::math::fvec<int, 2>(mons[1].work.x(), mons[1].work.y()),
			cgv::math::fvec<int, 2>(mons[1].work.x() + mons[1].work.w(), mons[1].work.y() + mons[1].work.h())));
	if ((ms & MS_MONITOR_3) != 0)
		rect.add_axis_aligned_box(cgv::media::axis_aligned_box<int, 2>(
			cgv::math::fvec<int, 2>(mons[2].work.x(), mons[2].work.y()),
			cgv::math::fvec<int, 2>(mons[2].work.x() + mons[2].work.w(), mons[2].work.y() + mons[2].work.h())));
	if ((ms & MS_MONITOR_4) != 0)
		rect.add_axis_aligned_box(cgv::media::axis_aligned_box<int, 2>(
			cgv::math::fvec<int, 2>(mons[3].work.x(), mons[3].work.y()),
			cgv::math::fvec<int, 2>(mons[3].work.x() + mons[3].work.w(), mons[3].work.y() + mons[3].work.h())));
	if (rect.is_valid()) {
		fltk::Monitor my_mon;
		my_mon.x(rect.get_min_pnt()[0]);
		my_mon.y(rect.get_min_pnt()[1]);
		my_mon.w(rect.get_extent()[0]);
		my_mon.h(rect.get_extent()[1]);
		fullscreen(my_mon);
	}
	else
		fullscreen();
}

/// set a different window state
void fltk_viewer_window::set_window_state(WindowState ws, MonitorSelection ms, bool update_control)
{
	if (ws == window_state && ms == fullscreen_monitors)
		return;
	// in case of previously minimized, recover window state before minimization
	if (window_state == WS_MINIMIZED) {
		fltk::Window::show();
		window_state = old_window_state;
		old_window_state = WS_REGULAR;
	}
	// in case of turning off fullscreen, recover window size and state before
	if ((window_state == WS_FULLSCREEN) && (ws != WS_FULLSCREEN)) {
		fullscreen_off(old_x, old_y, old_w, old_h);
		window_state = old_window_state;
	}
	if (window_state == WS_REGULAR) {
		old_x = x();
		old_y = y();
		old_w = w();
		old_h = h();
	}
	// finally switch to new window state
	switch (ws) {
	case WS_REGULAR:
		if (window_state != WS_REGULAR)
			resize(old_x, old_y, old_w, old_h);
		break;
	case WS_MINIMIZED:
		fltk::Window::iconize();
		break;
	case WS_MAXIMIZED:
		fltk::Window::maximize();
		break;
	case WS_FULLSCREEN :
		set_fullscreen(ms);
		break;
	}
//		switch (window_state) {
//		case WS_REGULAR:
//			break;
//		case WS_MINIMIZED:
//			show(false);
//			break;
//		case WS_MAXIMIZED:
//			resize(old_x, old_y, old_w, old_h);
//			break;
//		case WS_FULLSCREEN:
//			fullscreen_off(old_x, old_y, old_w, old_h);
//			break;
//		}

//		switch (ws) {
//		case WS_REGULAR :
//			show();
//			break;
//		case WS_MINIMIZED :
//			iconize();
//			break;
//		case WS_MAXIMIZED :
//			old_x = x();
//			old_y = y();
//			old_w = w();
//			old_h = h();
//			{
//				maximize();
//				//fltk::Rectangle r;
//				//borders(&r);
//				//const fltk::Monitor& m = fltk::Monitor::find(x(),y());
//				//resize(-r.x(),-r.y(),m.w()-r.w(),m.h()-r.h());
//			}
//			break;
//		case WS_FULLSCREEN:
//			old_x = x();
//			old_y = y();
//			old_w = w();
//			old_h = h();
//			{
//				const fltk::Monitor* mons;
//				int n = fltk::Monitor::list(&mons);
//				cgv::media::axis_aligned_box<int, 2> rect;
//				if ((ms & MS_MONITOR_1) != 0)
//					rect.add_axis_aligned_box(cgv::media::axis_aligned_box<int, 2>(
//						cgv::math::fvec<int, 2>(mons[0].work.x(), mons[0].work.y()),
//						cgv::math::fvec<int, 2>(mons[0].work.x() + mons[0].work.w(), mons[0].work.y() + mons[0].work.h())));
//				if ((ms & MS_MONITOR_2) != 0)
//					rect.add_axis_aligned_box(cgv::media::axis_aligned_box<int, 2>(
//						cgv::math::fvec<int, 2>(mons[1].work.x(), mons[1].work.y()),
//						cgv::math::fvec<int, 2>(mons[1].work.x() + mons[1].work.w(), mons[1].work.y() + mons[1].work.h())));
//				if ((ms & MS_MONITOR_3) != 0)
//					rect.add_axis_aligned_box(cgv::media::axis_aligned_box<int, 2>(
//						cgv::math::fvec<int, 2>(mons[2].work.x(), mons[2].work.y()),
//						cgv::math::fvec<int, 2>(mons[2].work.x() + mons[2].work.w(), mons[2].work.y() + mons[2].work.h())));
//				if ((ms & MS_MONITOR_4) != 0)
//					rect.add_axis_aligned_box(cgv::media::axis_aligned_box<int, 2>(
//						cgv::math::fvec<int, 2>(mons[3].work.x(), mons[3].work.y()),
//						cgv::math::fvec<int, 2>(mons[3].work.x() + mons[3].work.w(), mons[3].work.y() + mons[3].work.h())));
//				if (rect.is_valid()) {
//					fltk::Monitor my_mon;
//					my_mon.x(rect.get_min_pnt()[0]);
//					my_mon.y(rect.get_min_pnt()[1]);
//					my_mon.w(rect.get_extent()[0] );
//					my_mon.h(rect.get_extent()[1] );
//					fullscreen(my_mon);
//				}
//				else
//					fullscreen();
//			}
//			break;
//		}
//	}
	window_state = ws;
	fullscreen_monitors = ms;
	if (update_control) {
		update_member(&window_state);
		update_member(&fullscreen_monitors);
		redraw();
	}
}

void fltk_viewer_window::show_gui(bool update_control)
{
	if (gui_visible || tab_group->get_nr_children() == 0)
		return;

	gui_visible = true;
	main_group->dock(static_cast<fltk::Widget*>(tab_group->get_user_data()), 0, true);
	ensure_dock_state();
	ensure_dock_order();
	if (update_control && provider::find_control(gui_visible))
		provider::find_control(gui_visible)->update();
}
void fltk_viewer_window::show_menu(bool update_control)
{
	if (menu_visible || !menu)
		return;
	
	menu_visible = true;
	main_group->dock(menu, 1, false);
	ensure_dock_state();
	ensure_dock_order();
	if (update_control && provider::find_control(menu_visible))
		provider::find_control(menu_visible)->update();
}

bool fltk_viewer_window::gui_shown() const
{
	return gui_visible;// && (tab_group->get_nr_children() > 0);
}

bool fltk_viewer_window::menu_shown() const
{
	return menu_visible && menu;
}

void fltk_viewer_window::hide_gui(bool update_control)
{
	if (!gui_shown())
		return;
	
	gui_visible = false;
	main_group->undock(static_cast<fltk::Widget*>(tab_group->get_user_data()));
	ensure_dock_state();
	ensure_dock_order();
	if (update_control && provider::find_control(gui_visible))
		provider::find_control(gui_visible)->update();
}

void fltk_viewer_window::hide_menu(bool update_control)
{
	if (!menu_shown())
		return;

	menu_visible = false;
	main_group->undock(menu);
	ensure_dock_state();
	ensure_dock_order();
	if (update_control && provider::find_control(menu_visible))
		provider::find_control(menu_visible)->update();
}


/// return the current window state
WindowState fltk_viewer_window::get_window_state() const
{
	return window_state;
}

/*
struct provider_access : public cgv::gui::provider
{
	cgv::gui::gui_group_ptr get_parent() { return parent_group; }
	/// the gui window sets the parent group through this method
	void set_parent_public(cgv::gui::gui_group_ptr _parent) { set_parent(_parent); }
};
*/

void fltk_viewer_window::create_cb(fltk::Widget* w, void* _fac)
{
	fltk::ItemGroup* g = static_cast<fltk::ItemGroup*>(static_cast<fltk::Item*>(w)->parent());
	fltk_viewer_window* v = static_cast<fltk_viewer_window*>(g->user_data());
	
	factory* fac = static_cast<factory*>(_fac);

	// if we have a singleton factory
	if (fac->is_singleton_factory()) {
		// when singleton has been created
		if (!fac->get_singleton().empty()) {
			// toggle visibility and grab focus
			if (!v->get_view().empty()) {
				bool in_focus = v->get_view()->get_focus() == fac->get_singleton();
				cgv::render::drawable* d = 
					fac->get_singleton()->get_interface<cgv::render::drawable>();
				if (d) {
					if (in_focus || !d->get_active()) {
						d->set_active(!d->get_active());
						v->get_view()->post_redraw();
					}
				}
				if (!in_focus) {
					v->get_view()->set_focus(fac->get_singleton());
					cgv::gui::provider* p = 
						fac->get_singleton()->get_interface<cgv::gui::provider>();
					if (p) {
						cgv::gui::gui_group_ptr g = get_provider_parent(p);
						if (!g.empty())
							v->tab_group->select_child(g, true);
					}
				}
			}
		}
		// otherwise create singleton instance
		else
			cgv::base::register_object(fac->create_object(), fac->get_object_options());
	}
	else
		cgv::base::register_object(fac->create_object(), fac->get_object_options());
}

/// return a pointer to the main menu
fltk::Menu* fltk_viewer_window::get_menu()
{
	return menu;
}


/// handle on remove signals of the view to ensure that a singleton is dereferenced
void fltk_viewer_window::unregister_object(base_ptr c, const std::string& options)
{
	if (c->get_const_interface<cgv::gui::window>())
		return;
	// remove object from 3d view
	view->remove_child(c);

	// from tab group
	tab_group->unregister_object(c, options);
	if (tab_group->get_nr_children()==0) {
		main_group->undock(static_cast<fltk::Widget*>(tab_group->get_user_data()));
		ensure_dock_order();
	}

	// unregister singletons
	for (unsigned int i=0; i<factories.size(); ++i) {
		factory* f = factories[i]->get_interface<factory>();
		if (f->is_singleton_factory() && f->get_singleton() == c)
			f->release_singleton();
	}

	// remove menu entries
	provider* p = c->get_interface<cgv::gui::provider>();
	if (p) {
		std::string mp = p->get_menu_path();
		if (!mp.empty()) {
			if (mp.find_first_of('/') != std::string::npos)
				menu->remove(mp.c_str());
		}
	}
	update();
}

void fltk_viewer_window::menu_cb(fltk::Widget* w, void* obj_ptr)
{
	fltk::Group* g = w->parent();
	fltk_viewer_window* fvw = static_cast<fltk_viewer_window*>(g->user_data());
	base_ptr object(static_cast<cgv::base::base*>(obj_ptr));
	base_ptr gp = get_provider_parent(object->get_interface<provider>());
	fvw->tab_group->select_child(gp, true);
	for (unsigned ci=0; ci<fvw->view->get_nr_children(); ++ci)
		if (fvw->view->get_child(ci) == object) {
			fvw->view->get_interface<event_handler>()->set_focused_child((int)ci);
			break;
		}
}

void fltk_viewer_window::register_object(base_ptr object, const std::string& options)
{
	if (object->get_interface<cgv::gui::window>())
		return;
	// check for factories
	factory* f = object->get_interface<factory>();
	if (f) {
		factories.push_back(object);
		
		//const std::string& type_name = f->get_created_type_name();

		cgv::gui::shortcut sc;
		std::string item_text_str = f->get_created_type_name();
		
		base_generator bg;
		bg.add("shortcut", sc);
		bg.add("menu_text", item_text_str);
		bg.multi_set(options, false);

		int fltk_sc = fltk_shortcut(sc);
		if (fltk_sc > 0)
			fltk_sc |= fltk::COMMAND;

		char* item_text = new char[item_text_str.size()+1];
		strcpy(item_text, item_text_str.c_str());
		
		fltk::Widget* menu_item = menu->add(item_text, fltk_sc, create_cb, f);
		fltk::Group* g = static_cast<fltk::Group*>(menu_item->parent());
		ensure_menu_order();
		g->user_data(this);
		return;
	}

	std::string views;
	if (has_property(options, "views", views, false)) {
		if (cgv::utils::is_element(get_name(),views))
			view->append_child(object);
	}
	else {
		if ( (object->get_node() && !object->get_node()->get_parent()) || !object->get_node() )
			view->append_child(object);
	}

	std::string parents;
	if (has_property(options, "parents", parents, false))
		if (!cgv::utils::is_element(get_name(),parents))
			return;

	provider* p = object->get_interface<cgv::gui::provider>();
	if (p) {
		tab_group->register_object(object,options);
		// ensure docking of tab group
		show_gui();
		// add button to menu path
		std::string mp = p->get_menu_path();
		cgv::gui::shortcut sc = p->get_shortcut();

		base_generator bg;
		bg.add("menu_text", mp);
		bg.add("shortcut", sc);
		bg.multi_set(options, false);

		if (!mp.empty()) {
			if (!menu) {
				begin();
					menu = new fltk::MenuBar(0, 0, w(), 21);
					main_group->resize(0,21,w(),h()-21);
				end();
				init_sizes();
			}
			if (mp.find_first_of('/') != std::string::npos) {				
				fltk::ItemGroup* g1 = static_cast<fltk::ItemGroup*>(
					menu->add(mp.c_str(),fltk_shortcut(sc),menu_cb,object.operator->())->parent());
				g1->user_data(this);
				ensure_menu_order();
			}
		}
	} 
	update();
}

/// put default sizes into dimension fields and set inner_group to be active
void fltk_viewer_window::prepare_new_element(cgv::gui::gui_group_ptr ggp, int& x, int& y, int& w, int& h)
{
	x = 0;
	y = 0;
	w = 100;
	h = 100;
	main_group->begin();
}

/// align last element and add element to group
void fltk_viewer_window::finalize_new_element(cgv::gui::gui_group_ptr ggp, const std::string& _align, cgv::base::base_ptr element)
{
	main_group->end();
	cgv::base::group::append_child(element);
	int s = 0;
	bool res = false;
	if (_align.size() > 0 && _align[0] == 'R') 
		res = true;
	if (_align.size() > 0) {
		char l = _align[_align.size()-1];
		if (l >= '0' && l <= '3')
			s = l - '0';
	}
	fltk::Widget* element_widget = static_cast<fltk::Widget*>(element->get_user_data());
	if (element_widget->parent() != main_group)
		element_widget = element_widget->parent();
	main_group->dock(element_widget, s, res);
	ensure_dock_order();
}

