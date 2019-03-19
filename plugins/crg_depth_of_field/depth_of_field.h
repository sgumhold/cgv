#pragma once

#include <cgv/base/node.h>
#include <cgv/render/drawable.h>
#include <cgv/gui/provider.h>

#include "lib_begin.h"

class CGV_API depth_of_field : 
	public cgv::base::node, 
	public cgv::render::multi_pass_drawable,
	public cgv::gui::provider
{
protected:
	double z_focus;
	double aperture;
	int nr_samples;
	bool copy_z_focus;

	std::vector<double> samples;

	void put_sample(int i, double& dx, double& dy);
	void generate_samples();
	void update_view(cgv::render::context& ctx, int i);
	void restore_view(cgv::render::context& ctx, int i);
public:
	///
	depth_of_field();
	/// update gui and post redraw
	void on_set(void*);
	/// enum all member variables
	bool self_reflect(cgv::reflect::reflection_handler& rh);
	/// return the type name 
	std::string get_type_name() const;
	/// overload to show the content of this object
	void stream_stats(std::ostream&);
	///
	bool init(cgv::render::context& ctx);
	/// this method is called in one pass over all drawables before the draw method
	void init_frame(cgv::render::context&);
	/// this method is called in one pass over all drawables after drawing
	void finish_frame(cgv::render::context&);
	/// this method is called in one pass over all drawables after finish frame
	void after_finish(cgv::render::context&);
	/// return a shortcut to activate the gui without menu navigation
	cgv::gui::shortcut get_shortcut() const;
	/// return a path in the main menu to select the gui
	std::string get_menu_path() const;
	/// you must overload this for gui creation
	void create_gui();
};

#include <cgv/config/lib_end.h>
