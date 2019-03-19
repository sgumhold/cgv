#pragma once

#include <cgv/base/node.h>
#include <cgv/gui/event_handler.h>
#include <cgv/gui/provider.h>
#include <cgv/render/drawable.h>
#include <cgv_gl/surface_renderer.h>
#include <cgv/render/view.h>

/// example for the implementation of a cgv node that handles events and renders a cube
class grid : 
	public cgv::base::node,          
	public cgv::gui::event_handler,  /// derive from handler to receive events and to be asked for a help string
	public cgv::render::drawable,     /// derive from drawable for drawing the cube
	public cgv::gui::provider
{
protected:
	cgv::render::view* view_ptr;
	int minX,maxX,minZ,maxZ;
	void render_grid_lines(float alpha);
	vec3 z_axis;
	bool show_grid;
	bool show_lines;
	bool adaptive_grid;
	bool show_axes;
	float factor;
	float arrow_length;
	float arrow_aspect;
	float arrow_rel_tip_radius;
	float arrow_tip_aspect;
	float alpha;
	float threshold;
public:
	/// construct from name which is necessary construction argument to node
	grid();
	/// reflect adjustable members
	bool self_reflect(cgv::reflect::reflection_handler& srh);
	/// update gui and post redraw in on_set method
	void on_set(void* member_ptr);
	/// overload to return the type name of this object
	std::string get_type_name() const;
	/// optional method of base
	void stream_stats(std::ostream& os);
	/// necessary method of event_handler
	bool handle(cgv::gui::event& e);
	/// necessary method of event_handler
	void stream_help(std::ostream& os);
	/// 
	bool init(cgv::render::context&);
	/// optional method of drawable
	void finish_frame(cgv::render::context&);
	void draw_lines(cgv::render::context& ctx);
	void draw_grid(cgv::render::context&);
	void draw(cgv::render::context&);
	/// return a path in the main menu to select the gui
	std::string get_menu_path() const;
	/// you must overload this for gui creation
	void create_gui();
};
