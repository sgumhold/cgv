#pragma once

#include <cgv/render/textured_material.h>
#include <cgv/media/mesh/obj_loader.h>
#include <cgv/media/axis_aligned_box.h>
#include <cgv/render/drawable.h>

#include "lib_begin.h"

namespace cgv {
	/// namespace for api independent GPU programming
	namespace render {
		/// namespace for opengl specific GPU programming
		namespace gl {

/// extend textured_material with group-material assignments
struct CGV_API material_info : public cgv::render::textured_material
{
	/// for each group that uses this material the group index and the display list id
	std::vector<std::pair<unsigned,unsigned> > group_list_ids;
	/// construct from mtl info
	material_info(const cgv::media::illum::obj_material& i);
};

/// extend group_info by the total number of faces in the group
struct CGV_API mesh_group_info : public cgv::media::mesh::group_info
{
	/// total number of faces in the group
	unsigned number_faces;
	/// standard constructor
	mesh_group_info(const group_info& g);
};

/// simple implementation of a drawable for a polygonal mesh with support for materials
class CGV_API gl_mesh_drawable_base : public cgv::render::drawable
{
public:
	/// type of bounding box
	typedef cgv::media::axis_aligned_box<float,3> box_type;
protected:
	/// default path to file names
	std::string model_path;
	/// currently loaded file name
	std::string file_name;
	/// face material info
	std::vector<material_info> materials;
	/// group information
	std::vector<mesh_group_info> groups;
	/// the bounding box of the mesh is computed in the read_mesh method
	box_type box;
	/// the loader stores the mesh data in publicly accessible fields
	cgv::media::mesh::obj_loader loader;
	/// draw all faces belonging to the given group, optionally turn off the specification of materials
	void draw_mesh_group(cgv::render::context &ctx, unsigned gi, bool use_materials = true);
	/// draw the complete mesh, optionally turn off the specification of materials
	void draw_mesh(cgv::render::context &ctx, bool use_materials = true);
	//! call this to center the view after loading the mesh. 
	/*! For this to work you need to derive from at least cgv::base::node and register
	    an instance through the object registration mechanism defined in cgv/base/register.h */
	void center_view();
public:
	/// construct from name which is necessary construction argument to node
	gl_mesh_drawable_base();
	/// init textures if the mesh has been newly loaded
	void init_frame(cgv::render::context &ctx);
	/// default drawing implementation calls draw_mesh(ctx)
	void draw(cgv::render::context &ctx);
	/// read mesh from the given file_name which is extended by model path if it does not exist
	virtual bool read_mesh(const std::string& file_name);
	/// clear all objects living in the context like textures or display lists
	void clear(context& ctx);
	/// return the axis aligned bounding box
	const box_type& get_box() const;
};

		}
	}
}
#include <cgv/config/lib_end.h>