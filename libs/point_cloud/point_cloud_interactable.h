#pragma once

#include <cgv/base/group.h>
#include <cgv/gui/provider.h>
#include <cgv/gui/event_handler.h>
#include <cgv/gui/trigger.h>
#include <cgv/base/register.h>
#include "gl_point_cloud_drawable.h"
#include "ann_tree.h"
#include "neighbor_graph.h"
#include "normal_estimator.h"

#include "lib_begin.h"

enum PointCloudChangeEvent
{
	PCC_NEW_POINT_CLOUD        = 0x0001,
	PCC_POINTS                 = 0x0002,
	PCC_POINTS_RESIZE          = 0x0003,
	PCC_POINTS_MASK            = 0x0003,
							   
	PCC_COMPONENTS_CREATE      = 0x0004,
	PCC_COMPONENTS_RESIZE      = 0x0008,
	PCC_COMPONENTS_DESTRUCT    = 0x000C,
	PCC_COMPONENTS_MASK        = 0x000C,
							   
	PCC_NORMALS_CREATE         = 0x0010,
	PCC_NORMALS                = 0x0020,
	PCC_NORMALS_DESTRUCT       = 0x0030,
	PCC_NORMALS_MASK           = 0x0030,
							   
	PCC_COLORS_CREATE          = 0x0040,
	PCC_COLORS                 = 0x0080,
	PCC_COLORS_DESTRUCT        = 0x00C0,
	PCC_COLORS_MASK            = 0x00C0,
							   
	PCC_TEXCOORDS_CREATE       = 0x0100,
	PCC_TEXCOORDS              = 0x0200,
	PCC_TEXCOORDS_DESTRUCT     = 0x0300,
	PCC_TEXCOORDS_MASK         = 0x0300,
							   
	PCC_PIXCOORDS_CREATE       = 0x0400,
	PCC_PIXCOORDS              = 0x0800,
	PCC_PIXCOORDS_DESTRUCT     = 0x0C00,
	PCC_PIXCOORDS_MASK         = 0x0C00,

	PCC_NEIGHBORGRAPH_CREATE   = 0x1000,
	PCC_NEIGHBORGRAPH          = 0x2000,
	PCC_NEIGHBORGRAPH_DESTRUCT = 0x3000,
	PCC_NEIGHBORGRAPH_MASK     = 0x3000,

	PCC_WEIGHTS                = 0x4000,
	PCC_COMPONENT_TRANSFORMATION_CHANGE = 0x8000

};


/** the point cloud view adds a gui to the gl_point_cloud_drawable_base and adds
    some basic processing like normal computation as well as some debug rendering
	of the neighbor graph*/
class CGV_API point_cloud_interactable :
	public cgv::base::group,             // integration into the cgv::base::base hierarchy necessary for proper registration of instances, group type supports adding children 
	public gl_point_cloud_drawable,      // manages a point cloud and provides implementation of the cgv::render::drawable interface to render a point cloud
	public cgv::gui::event_handler,      // allows to handle key, mouse and vr events (under development) through the handle(event) method
	public cgv::gui::provider,           // allows to provide a gui through the create_gui() method
	public cgv::base::argument_handler   // allows to handle program arguments through the handle_args(args) method
{
protected:
	/**@name file io */
	//@{
	/// path of last opened or saved file
	std::string data_path;
	/// file name without path nor extension of current file
	std::string file_name;
private:
	/// helper member for updating the file name without overwriting the current one. This is needed for concatenation of file names when appending them
	std::string new_file_name;
protected:
	/// directory_name is used to support reading all point clouds from a directory by setting directory_path in the config file
	std::string directory_name;
	/// flag that tells whether reading a point cloud will append it to current or replace current
	bool do_append;
	/// whether to automatically set the view after reading new points
	bool do_auto_view;
	/// update data_path and file_name members from full file name
	void update_file_name(const std::string& ffn, bool append = false);
	/// save current point cloud to file with name fn
	bool save(const std::string& fn);
	/// open a new point cloud
	bool open(const std::string& fn);
	/// open a new point cloud by reading all point cloud files in given directory
	bool open_directory(const std::string& dn);
	/// open and append a new point cloud by reading file with name fn
	bool open_and_append(const std::string& fn);
	/// open or append depending on event that triggered this function
	bool open_or_append(cgv::gui::event& e, const std::string& file_name);
	//@}

	/**@name rendering speedup techniques */
	//@{
	/// allows to restrict rendering to a subrange of all points
	std::size_t show_point_start, show_point_count;
	/// allows to subsample point rendering during interaction in order to speed up interaction
	unsigned interact_point_step;
	/// delay in seconds waited before interaction subsampling is turned off again
	double interact_delay;
	/// trigger used to check for end of interaction subsampling
	cgv::gui::trigger interact_trigger;
	/// callback attached to interact_trigger
	void interact_callback(double t, double dt);
	/// current state of interaction subsampling
	enum InteractionState {
		IS_INTERMEDIATE_FRAME,
		IS_WAIT_INTERACTION_TO_STOP,
		IS_DRAW_FULL_FRAME,
		IS_FULL_FRAME
	} interact_state;
	//@}

	/**@name ann tree, neighbor graph and picking*/
	//@{
	/// whether ann tree needs rebuild
	bool tree_ds_out_of_date;
	/// the ann tree is used for nearest neighbor queries
	ann_tree* tree_ds;
	/// ensure that ann tree is built and current
	void ensure_tree_ds();
	/// k parameter for building neighbor graph
	unsigned k;
	/// whether to symmetric neighbor graph after build
	bool do_symmetrize;
	/// knn-neighbor graph built with tree_ds
	neighbor_graph ng;
	/// build the neighbor graph
	void build_neighbor_graph();
	/// build the neighbor graph
	void build_neighbor_graph_componentwise();
	/// normal estimation member
	normal_estimator ne;
	/// whether to use ann_tree to acceleration picking
	bool accelerate_picking;
	/// return the point closest to ray through given mouse position
	bool get_picked_point(int x, int y, unsigned& index);
	//@}

	/**@name normal computation and orientation*/
	//@{
	bool reorient_normals;
	void compute_normals();
	void recompute_normals();
	void toggle_normal_orientations();
	void orient_normals();
	void orient_normals_to_view_point();
	//@}

	/// pointer to instance that defines the view
	cgv::render::view* view_ptr;
	/// call this before using the view ptr for the first time
	bool ensure_view_pointer();

public:
	/// construct viewer with default configuration
	point_cloud_interactable();
	/// adjust view direction and extent at focus to bounding box of point cloud
	void auto_set_view();

	//**@name self reflection of class */
	//@{
	/// return type name of point_cloud_interactable
	std::string get_type_name() const;
	/// describe members
	bool self_reflect(cgv::reflect::reflection_handler& srh);
	/// stream out textual statistical information shown with F8
	void stream_stats(std::ostream&);
	/// stream out textual help information shown with F1
	void stream_help(std::ostream& os);
	//@}

	/**@name rendering */
	//@{
protected:
	/// whether to show neighbor graph
	bool show_neighbor_graph;
	/// function to activate edge color 
	void draw_edge_color(unsigned int vi, unsigned int j, bool is_symm, bool is_start) const;
	/// graph rendering function
	void draw_graph(cgv::render::context& ctx);
public:
	/// initialization on creation
	bool init(cgv::render::context& ctx);
	/// per frame initialization
	void init_frame(cgv::render::context& ctx);
	/// main rendering method
	void draw(cgv::render::context& ctx);
	//@}

	/**@name user interface */
	//@{
	/// helper function to ensure that limites of slides for point range selection are correct with respect to number of vertices in current point cloud
	void configure_subsample_controls();
	/// process command line arguments
	void handle_args(std::vector<std::string>& args);
	/// process key and mouse events
	bool handle(cgv::gui::event& e);
	/// used to update all dependent variables in case of changes to the point cloud
	virtual void on_point_cloud_change_callback(PointCloudChangeEvent pcc_event);
	/// used to update all dependent variables in case of changes to member variables
	void on_set(void* member_ptr);
	/// user interface creation
	void create_gui();
	//@}
};

typedef cgv::data::ref_ptr<point_cloud_interactable, true> point_cloud_viewer_ptr;

#include <cgv/config/lib_end.h>