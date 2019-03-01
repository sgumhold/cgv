#pragma once

#include <cgv/base/base.h>
#include <cgv/render/drawable.h>
#include <cgv/gui/event_handler.h>
#include <cgv/gui/provider.h>

#include "lib_begin.h"

class CGV_API antialias : 
	public cgv::base::node, 
	public cgv::gui::event_handler, 
	public cgv::render::drawable,
	public cgv::gui::provider
{
private:
	int iter;
	double pixel_scale_x, pixel_scale_y;
protected:
	int nr_samples;

	std::vector<double> samples;

	void put_sample(int i, double& dx, double& dy);
	void generate_samples();
	void update_view(cgv::render::context& ctx, int i);
	void restore_view(cgv::render::context& ctx, int i);
public:
	///
	antialias();
	/// update gui and post redraw
	void on_set(void*);
	/// enum all member variables
	bool self_reflect(cgv::reflect::reflection_handler& srh);
	/// return the type name 
	std::string get_type_name() const;
	/// overload to show the content of this object
	void stream_stats(std::ostream&);
	///
	bool init(cgv::render::context& ctx);
	/// overload and implement this method to handle events
	bool handle(cgv::gui::event& e);
	/// overload to stream help information to the given output stream
	void stream_help(std::ostream& os);
	/// this method is called in one pass over all drawables before the draw method
	void init_frame(cgv::render::context&);
	/// this method is called in one pass over all drawables after drawing
	void finish_frame(cgv::render::context&);
	/// this method is called in one pass over all drawables after finish frame
	void after_finish(cgv::render::context&);
	/// return a shortcut to activate the gui without menu navigation
//	cgv::gui::shortcut get_shortcut() const;
	/// return a path in the main menu to select the gui
//	std::string get_menu_path() const;
	/// you must overload this for gui creation
	void create_gui();
};

#include <cgv/config/lib_end.h>
