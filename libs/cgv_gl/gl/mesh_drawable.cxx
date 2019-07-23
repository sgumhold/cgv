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
	mesh_info.construct_vbos(ctx, mesh);
	mesh_info.bind(ctx, ctx.ref_surface_shader_program(true));
}

/// clear all objects living in the context like textures or display lists
void mesh_drawable::clear(context& ctx)
{
	mesh_info.destruct(ctx);
}

void mesh_drawable::render_mesh_group(context &ctx, shader_program& prog, unsigned gi, bool use_materials)
{
	for (size_t i = 0; i < mesh_info.get_nr_mesh_parts(); ++i)
		if (mesh_info.get_group_index(i) == gi)
			mesh_info.render_mesh_part(ctx, prog, i, true);
	for (size_t i = 0; i < mesh_info.get_nr_mesh_parts(); ++i)
		if (mesh_info.get_group_index(i) == gi)
			mesh_info.render_mesh_part(ctx, prog, i, false);
}

void mesh_drawable::render_mesh(context &ctx, shader_program& prog, bool use_materials)
{
	mesh_info.render_mesh(ctx, prog);
}

void mesh_drawable::draw(context &ctx)
{
	render_mesh(ctx, ctx.ref_surface_shader_program(true));
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

