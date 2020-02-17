#include <cgv/base/base.h>
#include "mesh_drawable.h"
#include <cgv/base/import.h>

namespace cgv {
	namespace render {
		namespace gl {

/// construct from name which is necessary construction argument to node
mesh_drawable::mesh_drawable()
{
	rebuild_mesh_info = false;
}

/// init textures
void mesh_drawable::init_frame(context &ctx)
{
	if (!rebuild_mesh_info)
		return;

	mesh_info.destruct(ctx);
	mesh_info.construct(ctx, mesh);
	mesh_info.bind(ctx, ctx.ref_surface_shader_program(true), true);
}

/// clear all objects living in the context like textures or display lists
void mesh_drawable::clear(context& ctx)
{
	mesh_info.destruct(ctx);
}

void mesh_drawable::draw_mesh_primitive(context &ctx, unsigned pi, bool opaque, bool use_materials)
{
	mesh_info.draw_primitive(ctx, pi, !opaque, opaque, use_materials);
}

void mesh_drawable::draw_mesh(context &ctx, bool opaque, bool use_materials)
{
	mesh_info.draw_all(ctx, !opaque, opaque);
}

void mesh_drawable::draw(context &ctx)
{
	draw_mesh(ctx, true);
}

void mesh_drawable::finish_draw(context &ctx)
{
	draw_mesh(ctx, false);
}

bool mesh_drawable::read_mesh(const std::string& _file_name)
{
	std::string fn = cgv::base::find_data_file(_file_name, "cpMD", "", model_path);
	if (fn.empty()) {
		std::cerr << "mesh file " << file_name << " not found" << std::endl;
		return false;
	}
	file_name = _file_name;
	mesh.clear();
	if (mesh.read(file_name)) {
		rebuild_mesh_info = true;
		return true;
	}
	else {
		std::cerr << "could not read mesh " << file_name << std::endl;
		return false;
	}
	box = mesh.compute_box();
}

void mesh_drawable::center_view()
{
	cgv::render::view* view_ptr = find_view_as_node();
	if (!view_ptr)
		return;
	view_ptr->set_focus(box.get_center());
	view_ptr->set_y_extent_at_focus(1.3*box.get_extent()(box.get_max_extent_coord_index()));
	post_redraw();
}
/// return the axis aligned bounding box
const mesh_drawable::box3& mesh_drawable::get_box() const
{
	return box;
}


		}
	}
}

