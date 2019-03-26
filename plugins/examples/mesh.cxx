#include <cgv/base/node.h>
#include <cgv/defines/quote.h>
#include <cgv/render/shader_program.h>
#include <cgv/render/drawable.h>
#include <cgv/render/clipped_view.h>
#include <cgv_gl/gl/gl.h>
#include <cgv_gl/gl/mesh_render_info.h>
#include <cgv_gl/gl/gl_context.h>
#include <cgv/render/attribute_array_binding.h>
#include <cgv/gui/provider.h>
#include <cgv/media/mesh/simple_mesh.h>
#include <cgv/gui/key_event.h>
#include <cgv/gui/provider.h>
#include <cgv/math/ftransform.h>

using namespace cgv::base;
using namespace cgv::signal;
using namespace cgv::gui;
using namespace cgv::math;
using namespace cgv::render;
using namespace cgv::utils;
using namespace cgv::media::illum;

class mesh : public node, public drawable, public provider
{
private:
	bool have_new_mesh;
public:
	typedef cgv::media::mesh::simple_mesh<float> mesh_type;
	typedef mesh_type::idx_type idx_type;
	typedef mesh_type::vec3i vec3i;

	bool show_surface;
	CullingMode cull_mode;
	MaterialSide color_mapping;
	rgb  surface_color;
	IlluminationMode illumination_mode;

	bool show_wireframe;
	float line_width;
	rgb  line_color;

	std::string file_name;

	mesh_type M;
	cgv::render::mesh_render_info mesh_info;
public:
	mesh() : node("mesh")
	{
		show_surface = true;
		cull_mode = CM_BACKFACE;
		color_mapping = MS_NONE;
		surface_color = rgb(0.7f, 0.2f, 1.0f);
		illumination_mode = IM_ONE_SIDED;

		show_wireframe = false;
		line_width = 2.0f;
		line_color = rgb(0.6f,0.5f,0.4f);
		M.read(QUOTE_SYMBOL_VALUE(INPUT_DIR) "/res/example.obj");
		file_name = "example.obj";
		have_new_mesh = true;
	}
	void on_set(void* member_ptr)
	{
		if (member_ptr == &file_name) {
			mesh_type tmp;
			if (tmp.read(file_name)) {
				M = tmp;
				have_new_mesh = true;
			}
		}
		update_member(member_ptr);
		post_redraw();
	}
	void create_gui()
	{
		add_decorator("mesh", "heading", "level=2");
		add_gui("file_name", file_name, "file_name", 
			    "title='open obj file';filter='mesh (obj):*.obj|all files:*.*'");

		add_member_control(this, "surface", show_surface, "toggle", "shortcut='s'");
		add_member_control(this, "cull mode", cull_mode, "dropdown", "enums='none,back,front'");
		add_member_control(this, "color mapping", color_mapping, "dropdown", "enums='none,front,back,front+back'");
		add_member_control(this, "surface color", surface_color);
		add_member_control(this, "illumination", illumination_mode, "dropdown", "enums='none,one sided,two sided'");

		add_member_control(this, "wireframe", show_wireframe, "toggle", "shortcut='w'");
		add_member_control(this, "line width", line_width, "value_slider", "min=1;max=20;ticks=true;log=true");
		add_member_control(this, "line color", line_color);

		for (unsigned mi = 0; mi < mesh_info.mesh_mats.size(); ++mi) {
			if (begin_tree_node(mesh_info.mesh_mats[mi]->get_name(), *mesh_info.mesh_mats[mi])) {
				align("\a");
				add_gui("mat", static_cast<cgv::media::illum::textured_surface_material&>(*mesh_info.mesh_mats[mi]));
				align("\b");
				end_tree_node(*mesh_info.mesh_mats[mi]);
			}
		}
	}
	void init_frame(context& ctx)
	{
		if (have_new_mesh) {
			if (!M.has_normals())
				M.compute_vertex_normals();
			mesh_info.destruct(ctx);
			mesh_info.construct(ctx, M);
			post_recreate_gui();
			have_new_mesh = false;
			clipped_view* view_ptr = dynamic_cast<clipped_view*>(find_view_as_node());
			if (view_ptr) {
				view_ptr->set_scene_extent(M.compute_box());
				view_ptr->set_default_view();
			}
		}
	}
	void draw(context& ctx)
	{
		if (show_wireframe) {
			GLfloat old_line_width;
			glGetFloatv(GL_LINE_WIDTH, &old_line_width);
			glLineWidth(line_width);
				ctx.ref_default_shader_program().enable(ctx);
					ctx.set_color(line_color);
					mesh_info.render_wireframe(ctx);
				ctx.ref_default_shader_program().disable(ctx);
			glLineWidth(old_line_width);
		}
		if (show_surface) {			
			GLboolean is_culling = glIsEnabled(GL_CULL_FACE);
			GLint cull_face;
			glGetIntegerv(GL_CULL_FACE_MODE, &cull_face);

			if (cull_mode > 0) {
				glEnable(GL_CULL_FACE);
				glCullFace(cull_mode == CM_BACKFACE ? GL_BACK : GL_FRONT);
			}
			else
				glDisable(GL_CULL_FACE);

			shader_program& prog = ctx.ref_surface_shader_program(true);
			prog.enable(ctx);
			ctx.set_color(surface_color);
			prog.set_uniform(ctx, "culling_mode", (int)cull_mode);
			prog.set_uniform(ctx, "map_color_to_material", (int)color_mapping);
			prog.set_uniform(ctx, "illumination_mode", (int)illumination_mode);
			prog.disable(ctx);
			mesh_info.render_mesh(ctx);
			
			if (is_culling)
				glEnable(GL_CULL_FACE);
			else
				glDisable(GL_CULL_FACE);
			glCullFace(cull_face);
		}
	}
};

#include <cgv/base/register.h>

/// register a factory to create new cubes
extern cgv::base::factory_registration<mesh> mesh_fac("new/media/mesh", 'M');
