#pragma once

#include <cgv/base/node.h>
#include <cgv/signal/abst_signal.h>	/// the tacker class is declared in abst_signal 
#include <cgv/gui/event_handler.h>
#include <cgv/render/drawable.h>
#include <cgv_gl/gl/mesh_render_info.h>
#include <cgv/render/attribute_array_binding.h>
#include <cgv/gui/provider.h>
#include <cgv/media/mesh/simple_mesh.h>



/// example for the implementation of a cgv node that handles events and renders a selectable shape
class shape : 
	public cgv::base::group,          /// derive from node to integrate into global tree structure and to store a name
	public cgv::gui::event_handler,  /// derive from handler to receive events and to be asked for a help string
	public cgv::render::drawable,    /// derive from drawable for drawing the cube
	public cgv::gui::provider
{
private:
	/// flag used to store state of collapsable gui node
	bool node_flag;
	bool show_edges;
	bool show_faces;
protected:
	/// resolution of smooth shapes
	int resolution;
	/// different shape types
	enum Shape { CUBE, PRI, TET, OCT, DOD, ICO, CYL, CONE, DISK, ARROW, SPHERE, MESH } shp;
	/// store rotation angle
	double ax, ay;
	/// store location along x-axis
	double x;
	///
	cgv::media::illum::surface_material mat;
	///
	cgv::media::illum::surface_material::color_type col;
	///
	typedef cgv::media::mesh::simple_mesh<float> mesh_type;
	typedef mesh_type::idx_type idx_type;
	typedef mesh_type::vec3i vec3i;
	mesh_type mesh;
	cgv::render::mesh_render_info mesh_info;

	/// whether to flip the normals
	bool flip_normals;
	/// select a shape
	void select_shape(Shape s);
public:
	void on_set(void*);
	/// construct from name which is necessary construction argument to node
	shape(const char* name);
	/// optional method of base
	void stream_stats(std::ostream& os);
	/// necessary method of event_handler
	bool handle(cgv::gui::event& e);
	/// necessary method of event_handler
	void stream_help(std::ostream& os);
	/// optional method of drawable
	void draw_shape(cgv::render::context&, bool);
	///
	bool init(cgv::render::context&);
	/// optional method of drawable
	void draw(cgv::render::context&);
	/// return a path in the main menu to select the gui
	std::string get_menu_path() const;
	/// you must overload this for gui creation
	void create_gui();
};
