#pragma once

#include <cgv/render/textured_material.h>
#include <cgv/render/vertex_buffer.h>
#include <cgv/render/attribute_array_binding.h>
#include <cgv/media/mesh/simple_mesh.h>
#include <cgv/media/axis_aligned_box.h>
#include <cgv/render/drawable.h>
#include "mesh_render_info.h"

#include "lib_begin.h"

namespace cgv {
	/// namespace for api independent GPU programming
	namespace render {
		/// namespace for opengl specific GPU programming
		namespace gl {


/// simple implementation of a drawable for a polygonal mesh with support for materials
class CGV_API mesh_drawable : public cgv::render::drawable
{
public:
	/// type of mesh
	typedef cgv::media::mesh::simple_mesh<float> mesh_type;
protected:
	/// default path to file names
	std::string model_path;
	/// currently loaded file name
	std::string file_name;
	///
	mesh_type mesh;
	///
	mesh_render_info mesh_info;
	///
	bool rebuild_mesh_info;
	/// the bounding box of the mesh is computed in the read_mesh method
	box3 box;
	/// draw all faces belonging to the given primitive, optionally turn off the specification of materials
	void draw_mesh_primitive(cgv::render::context &ctx, unsigned pi, bool opaque, bool use_materials = true);
	/// draw the complete mesh, optionally turn off the specification of materials
	void draw_mesh(cgv::render::context &ctx, bool opaque, bool use_materials = true);
	//! call this to center the view after loading the mesh. 
	/*! For this to work you need to derive from at least cgv::base::node and register
	    an instance through the object registration mechanism defined in cgv/base/register.h */
	void center_view();
public:
	/// construct from name which is necessary construction argument to node
	mesh_drawable();
	/// init textures if the mesh has been newly loaded
	void init_frame(cgv::render::context &ctx);
	/// default drawing implementation calls draw_mesh(ctx, true) to render opaque part
	void draw(cgv::render::context &ctx);
	/// default finish drawing implementation calls draw_mesh(ctx, false) to render transparent part
	void finish_draw(context &ctx);
	/// read mesh from the given file_name which is extended by model path if it does not exist
	virtual bool read_mesh(const std::string& file_name);
	/// clear all objects living in the context like textures or display lists
	void clear(context& ctx);
	/// return the axis aligned bounding box
	const box3& get_box() const;
};

		}
	}
}
#include <cgv/config/lib_end.h>