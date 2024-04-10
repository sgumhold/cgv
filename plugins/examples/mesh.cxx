#include <cgv/base/node.h>
#include <cgv/defines/quote.h>
#include <cgv/render/shader_program.h>
#include <cgv/render/drawable.h>
#include <cgv/render/clipped_view.h>
#include <cgv_gl/gl/gl.h>
#include <cgv_gl/sphere_renderer.h>
#include <cgv_gl/cone_renderer.h>
#include <cgv_gl/gl/mesh_render_info.h>
#include <cgv_gl/gl/gl_context.h>
#include <cgv/render/attribute_array_binding.h>
#include <cgv/gui/provider.h>
#include <cgv/media/color_scale.h>
#include <cgv/media/mesh/simple_mesh.h>
#include <cgv/gui/key_event.h>
#include <cgv/gui/provider.h>
#include <cgv/gui/dialog.h>
#include <cgv/math/ftransform.h>

using namespace cgv::base;
using namespace cgv::signal;
using namespace cgv::gui;
using namespace cgv::math;
using namespace cgv::render;
using namespace cgv::utils;
using namespace cgv::media::illum;

/*
This example illustrates how to use cgv::mesh::simple_mesh<T> and cgv::render::mesh_render_info
to render a polygonal mesh in potentially textured surface rendering mode and in wireframe mode.
The mesh_render_info manages vertex buffers for position, normal, tex_coord and color attributes
and an element index buffer for triangle based surface and edge based wireframe rendering. 
Furthermore, an attribute_array_binding object is managed.

The typical workflow is

PREPARATION (implemented in init or init_frame method of drawable)
- construct simple_mesh data structure with new_position(), new_normal(), new_tex_coord(), 
  start_face() and new_corner() methods. If also per vertex colors are needed, allocate
  them with dynamically choosable type through the ensure_colors() method of the base
  class cgv::media::colored_model and the same number of colors as you have positions.
  Then individual colors can be set with the set_color() functions and retreived later on
  with the put_color() functions. 
  An alternative is to read a simple mesh from an obj-file with the read() method.
  Optionally, you can compute vertex normals with the compute_vertex_normals() method after
  construction of positions, faces and corners.
  [this step can be implemented also outside of the drawable methods as no render context is needed]
- construct the vertex buffers of the mesh_render_info object from the simple_mesh with its 
  construct_vbos() method
- choose a shader program and bind the attribute_array_binding of the mesh_render_info struct to
  the attribute locations of the program with the bind() method. This will also bind the element
  index buffer.

RENDERING PHASE (implemented in draw and finish_draw method)
- configure uniforms and constant vertex attributes of shader program 
- call render_mesh() method of mesh_render_info and pass chosen shader program to it.
  This will enable and disable the shader program as needed. render_mesh() has two optional
  boolean parameters that allow to ignore transparent or opaque mesh parts during rendering.
  If the mesh has transparent parts, one typically renders first all opaque parts and in a 
  second pass or in the finish_draw method all transparent parts. This will not produce a 
  correct visibility ordering of the transparent parts but will blend over the transparent
  parts over the opaque parts. If not done like this, transparent parts can occlude opaque
  parts due to the z-buffer algorithm used for visibility sorting.

The mesh_render_info class provides part based rendering methods for meshes with several
materials and several groups as supported by the obj file format with the render_mesh_part()
method. Furthermore, there are two draw methods draw_surface() and draw_wireframe(). Both 
methods do not enable any shader program and assume the vertex attribute locations of the
used shader program to be the same as of the shader program passed to the bind() method in
the preparation stage. In this example the draw_wireframe() method is used for wireframe 
rendering.

The example furthermore supports reading of obj files and can serve as simple mesh viewer.
Some image formats used for 3d models are not support by the cmi_io plugin used by default
in the examples plugin. More image formats are provided by the the cmi_devIL plugin which is
only part of the cgv_support project tree available on demand.
*/

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
	ColorMapping color_mapping;
	cgv::rgb surface_color;
	IlluminationMode illumination_mode;

	bool show_vertices;
	sphere_render_style sphere_style;

	bool sphere_aam_out_of_date;
	attribute_array_manager sphere_aam;

	bool show_wireframe;
	cone_render_style cone_style;
	attribute_array_manager cone_aam;

	std::string file_name;

	mesh_type M;

	cgv::render::mesh_render_info mesh_info;

	// mesh generation parameters
	int n, m;
	float a, b;
	float lb, ub;
public:
	mesh() : node("mesh")
	{
		show_surface = true;
		cull_mode = CM_BACKFACE;
		color_mapping = cgv::render::CM_COLOR;
		surface_color = cgv::rgb(0.7f, 0.2f, 1.0f);
		illumination_mode = IM_ONE_SIDED;

		show_wireframe = true;

		show_vertices = true;
		if (getenv("CGV_DIR"))
			M.read(std::string(getenv("CGV_DIR")) + "/plugins/examples/res/example.obj");
		else
			M.construct_conway_polyhedron("adtD");
		sphere_style.radius = float(0.05*sqrt(M.compute_box().get_extent().sqr_length() / M.get_nr_positions()));
		sphere_style.surface_color = cgv::rgb(0.8f, 0.3f, 0.3f);
		
		cone_style.radius = 0.5f*sphere_style.radius;
		cone_style.surface_color = cgv::rgb(0.6f, 0.5f, 0.4f);

		sphere_aam_out_of_date = true;
		
		file_name = "example.obj";
		have_new_mesh = true;

		a = 1;
		b = 0.2f;
		lb = 0.01f;
		ub = 2.0f;
		n = m = 20;
	}
	bool on_exit_request()
	{
		return cgv::gui::question("Are you sure to leave mesh view?", "Yes,No,Cancel", 2) == 0;
	}
	void generate_dini_surface()
	{
		M.clear();
		// allocate per vertex colors of type rgb with float components
		M.ensure_colors(cgv::media::CT_RGB, (n+1)*m);

		for (int i = 0; i <= n; ++i) {
			float y = (float)i / n;
			float v = (ub-lb)*y + lb;
			for (int j = 0; j < m; ++j) {
				float x = (float)j / m;
				float u = float(4.0f*M_PI)*x;
				// add new position to the mesh (function returns position index, which is i*m+j in our case)
				int vi = M.new_position(cgv::vec3(a*cos(u)*sin(v), a*sin(u)*sin(v), a*(cos(v) + log(tan(0.5f*v))) + b * u));
				// set color
				M.set_color(vi, cgv::rgb(x, y, 0.5f));
				// add quad connecting current vertex with previous ones
				if (i > 0) {
					int vi = ((i-1) * m + j);
					int delta_j = -1;
					if (j == 0)
						delta_j = m - 1;
					M.start_face();
					M.new_corner(vi);
					M.new_corner(vi + m);
					M.new_corner(vi + m + delta_j);
					M.new_corner(vi + delta_j);
				}
			}
		}
		// compute surface normals at mesh vertices from quads
		M.compute_vertex_normals();
		
		have_new_mesh = true;
		post_redraw();
	}
	void on_set(void* member_ptr)
	{
		if (member_ptr == &file_name) {
			mesh_type tmp;
			tmp.ensure_colors(cgv::media::CT_RGB);
			if (tmp.read(file_name)) {
				if (tmp.get_nr_colors() == 0)
					tmp.destruct_colors();
				M = tmp;
				sphere_style.radius = float(0.05*sqrt(M.compute_box().get_extent().sqr_length() / M.get_nr_positions()));
				on_set(&sphere_style.radius);
				cone_style.radius = 0.5f*sphere_style.radius;
				on_set(&cone_style.radius);
				if (cgv::utils::file::get_file_name(file_name) == "Max-Planck_lowres.obj")
					construct_mesh_colors();
				have_new_mesh = true;
			}
		}
		update_member(member_ptr);
		post_redraw();
	}
	// a hack that adds vertex colors to a mesh and used for illustration purposes only
	void construct_mesh_colors()
	{
		if (M.has_colors())
			return;
		M.ensure_colors(cgv::media::CT_RGB, M.get_nr_positions());
		double int_part;
		for (unsigned i = 0; i < M.get_nr_positions(); ++i)			
			M.set_color(i, cgv::media::color_scale(modf(20*double(i)/(M.get_nr_positions() - 1),&int_part)));
	}
	void create_gui()
	{
		add_decorator("mesh", "heading", "level=2");
		add_gui("file_name", file_name, "file_name", 
			    "title='open obj file';filter='mesh (obj):*.obj|all files:*.*';w=140");

		bool show = begin_tree_node("generate", a, false, "options='w=140';align=' '");
		connect_copy(add_button("generate", "w=52")->click, cgv::signal::rebind(this, &mesh::generate_dini_surface));
		if (show) {
			align("\a");
			add_member_control(this, "a", a, "value_slider", "min=0.1;max=10;ticks=true;log=true");
			add_member_control(this, "b", b, "value_slider", "min=0.1;max=10;ticks=true;log=true");
			add_member_control(this, "lb", lb, "value_slider", "min=0.001;step=0.0001;max=1;ticks=true;log=true");
			add_member_control(this, "ub", ub, "value_slider", "min=1;max=10;ticks=true;log=true");
			add_member_control(this, "n", n, "value_slider", "min=5;max=100;ticks=true;log=true");
			add_member_control(this, "m", m, "value_slider", "min=5;max=100;ticks=true;log=true");
			align("\b");
			end_tree_node(a);
		}
		show = begin_tree_node("vertices", show_vertices, false, "options='w=100';align=' '");
		add_member_control(this, "show", show_vertices, "toggle", "w=42;shortcut='w'", " ");
		add_member_control(this, "", sphere_style.surface_color, "", "w=42");
		if (show) {
			align("\a");
			add_gui("style", sphere_style);
			align("\b");
			end_tree_node(show_wireframe);
		}
		show = begin_tree_node("wireframe", show_wireframe, false, "options='w=100';align=' '");
		add_member_control(this, "show", show_wireframe, "toggle", "w=42;shortcut='w'", " ");
		add_member_control(this, "", cone_style.surface_color, "", "w=42");
		if (show) {
			align("\a");
			add_gui("style", cone_style);
			align("\b");
			end_tree_node(show_wireframe);
		}

		show = begin_tree_node("surface", show_surface, false, "options='w=100';align=' '");
		add_member_control(this, "show", show_surface, "toggle", "w=42;shortcut='s'", " ");
		add_member_control(this, "", surface_color, "", "w=42");
		if (show) {
			align("\a");
			add_member_control(this, "cull mode", cull_mode, "dropdown", "enums='none,back,front'");
			if (begin_tree_node("color_mapping", color_mapping)) {
				align("\a");
				add_gui("color mapping", color_mapping, "bit_field_control",
					"enums='COLOR_FRONT=1,COLOR_BACK=2,OPACITY_FRONT=4,OPACITY_BACK=8'");
				align("\b");
				end_tree_node(color_mapping);
			}
			add_member_control(this, "surface color", surface_color);
			add_member_control(this, "illumination", illumination_mode, "dropdown", "enums='none,one sided,two sided'");
			// this is how to add a ui for the materials read from an obj material file
			for (unsigned mi = 0; mi < mesh_info.ref_materials().size(); ++mi) {
				if (begin_tree_node(mesh_info.ref_materials()[mi]->get_name(), *mesh_info.ref_materials()[mi])) {
					align("\a");
					add_gui("mat", static_cast<cgv::media::illum::textured_surface_material&>(*mesh_info.ref_materials()[mi]));
					align("\b");
					end_tree_node(*mesh_info.ref_materials()[mi]);
				}
			}
			align("\b");
			end_tree_node(show_surface);
		}
	}
	bool init(context& ctx)
	{
		ref_sphere_renderer(ctx, 1);
		ref_cone_renderer(ctx, 1);
		sphere_aam.init(ctx);
		cone_aam.init(ctx);
		return true;
	}
	void clear(context& ctx)
	{
		ref_cone_renderer(ctx, -1);
		ref_sphere_renderer(ctx, -1);
		sphere_aam.destruct(ctx);
		cone_aam.destruct(ctx);
	}
	void init_frame(context& ctx)
	{
		if (have_new_mesh) {
			// auto-compute mesh normals if not available
			if (!M.has_normals())
				M.compute_vertex_normals();
			// [re-]compute mesh render info
			mesh_info.destruct(ctx);
			mesh_info.construct(ctx, M);
			// bind mesh attributes to standard surface shader program
			mesh_info.bind(ctx, ctx.ref_surface_shader_program(true), true);
			mesh_info.bind_wireframe(ctx, ref_cone_renderer(ctx).ref_prog(), true);
			// ensure that materials are presented in gui
			post_recreate_gui();
			have_new_mesh = false;
			sphere_aam_out_of_date = true;
			
			// focus view on new mesh
			clipped_view* view_ptr = dynamic_cast<clipped_view*>(find_view_as_node());
			if (view_ptr) {
				cgv::box3 box = M.compute_box();
				view_ptr->set_scene_extent(box);
				view_ptr->set_focus(box.get_center());
				view_ptr->set_y_extent_at_focus(box.get_extent().length());
			}
		}
	}
	void draw_surface(context& ctx, bool opaque_part)
	{
		// remember current culling setting
		GLboolean is_culling = glIsEnabled(GL_CULL_FACE);
		GLint cull_face;
		glGetIntegerv(GL_CULL_FACE_MODE, &cull_face);

		// ensure that opengl culling is identical to shader program based culling
		if (cull_mode > 0) {
			glEnable(GL_CULL_FACE);
			glCullFace(cull_mode == CM_BACKFACE ? GL_BACK : GL_FRONT);
		}
		else
			glDisable(GL_CULL_FACE);

		// choose a shader program and configure it based on current settings
		shader_program& prog = ctx.ref_surface_shader_program(true);
		prog.set_uniform(ctx, "culling_mode", (int)cull_mode);
		prog.set_uniform(ctx, "map_color_to_material", (int)color_mapping);
		prog.set_uniform(ctx, "illumination_mode", (int)illumination_mode);
		// set default surface color for color mapping which only affects 
		// rendering if mesh does not have per vertex colors and color_mapping is on
		//prog.set_attribute(ctx, prog.get_color_index(), surface_color);
		ctx.set_color(surface_color);
		// render the mesh from the vertex buffers with selected program
		mesh_info.draw_all(ctx, opaque_part, !opaque_part);

		// recover opengl culling mode
		if (is_culling)
			glEnable(GL_CULL_FACE);
		else
			glDisable(GL_CULL_FACE);
		glCullFace(cull_face);
	}

	void draw(context& ctx)
	{
		if (show_vertices) {
			sphere_renderer& sr = ref_sphere_renderer(ctx);
			sr.set_render_style(sphere_style);
			sr.enable_attribute_array_manager(ctx, sphere_aam);
			if (sphere_aam_out_of_date) {
				sr.set_position_array(ctx, M.get_positions());
				if (M.has_colors())
					sr.set_color_array(ctx, *reinterpret_cast<const std::vector<cgv::rgb>*>(M.get_color_data_vector_ptr()));
				sphere_aam_out_of_date = false;
			}
			sr.render(ctx, 0, M.get_nr_positions());
			sr.disable_attribute_array_manager(ctx, sphere_aam);
		}
		if (show_wireframe) {
			cone_renderer& cr = ref_cone_renderer(ctx);
			cr.set_render_style(cone_style);
			if (cr.enable(ctx)) {
				mesh_info.draw_wireframe(ctx);
				cr.disable(ctx);
			}
		}
		if (show_surface) {
			draw_surface(ctx, true);
		}
	}
	void finish_frame(context& ctx)
	{
		if (show_surface)
			draw_surface(ctx, false);
	}
};

#include <cgv/base/register.h>

/// register a factory to create new cubes
cgv::base::factory_registration<mesh> mesh_fac("New/Media/Mesh", 'M');
