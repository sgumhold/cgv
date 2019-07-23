#include <cgv/base/node.h>
#include <cgv/defines/quote.h>
#include <cgv/render/shader_program.h>
#include <cgv/render/drawable.h>
#include <cgv/render/clipped_view.h>
#include <cgv_gl/gl/gl.h>
#include <cgv_gl/sphere_renderer.h>
#include <cgv_gl/gl/mesh_render_info.h>
#include <cgv_gl/gl/gl_context.h>
#include <cgv/render/attribute_array_binding.h>
#include <cgv/gui/provider.h>
#include <cgv/media/color_scale.h>
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

	bool show_vertices;
	cgv::render::sphere_render_style sphere_style;

	std::string file_name;

	mesh_type M;

	cgv::render::mesh_render_info mesh_info;

	// mesh generation parameters
	size_t n, m;
	float a, b;
	float lb, ub;
public:
	mesh() : node("mesh")
	{
		show_surface = true;
		cull_mode = CM_BACKFACE;
		color_mapping = MS_FRONT_AND_BACK;
		surface_color = rgb(0.7f, 0.2f, 1.0f);
		illumination_mode = IM_ONE_SIDED;

		show_wireframe = true;
		line_width = 2.0f;
		line_color = rgb(0.6f,0.5f,0.4f);

		show_vertices = true;
		M.read(QUOTE_SYMBOL_VALUE(INPUT_DIR) "/res/example.obj");
		sphere_style.radius = float(0.05*sqrt(M.compute_box().get_extent().sqr_length() / M.get_nr_positions()));
		file_name = "example.obj";
		have_new_mesh = true;

		a = 1;
		b = 0.2f;
		lb = 0.01f;
		ub = 2.0f;
		n = m = 20;
	}
	void generate_dini_surface()
	{
		M.clear();
		// allocate per vertex colors of type rgb with float components
		M.ensure_colors(cgv::media::CT_RGB, (n+1)*m);

		for (size_t i = 0; i <= n; ++i) {
			float y = (float)i / n;
			float v = (ub-lb)*y + lb;
			for (size_t j = 0; j < m; ++j) {
				float x = (float)j / m;
				float u = float(4.0f*M_PI)*x;
				// add new position to the mesh (function returns position index, which is i*(m+1)+j in our case)
				int vi = M.new_position(vec3(a*cos(u)*sin(v), a*sin(u)*sin(v), a*(cos(v) + log(tan(0.5f*v))) + b * u));
				// set color
				M.set_color(vi, rgb(x, y, 0.5f));
				// add quad connecting current vertex with previous ones
				if (i > 0) {
					int vi = (i-1) * m + j;
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
			if (tmp.read(file_name)) {
				M = tmp;
				sphere_style.radius = float(0.05*sqrt(M.compute_box().get_extent().sqr_length() / M.get_nr_positions()));
				on_set(&sphere_style.radius);
				if (cgv::utils::file::get_file_name(file_name) == "Max-Planck_lowres.obj")
					construct_mesh_colors();
				have_new_mesh = true;
			}
		}
		update_member(member_ptr);
		post_redraw();
	}
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
		show = begin_tree_node("vertex spheres", show_vertices, false, "options='w=140';align=' '");
		add_member_control(this, "show", show_vertices, "toggle", "w=52;shortcut='w'");
		if (show) {
			align("\a");
			add_gui("style", sphere_style);
			align("\b");
			end_tree_node(show_wireframe);
		}

		show = begin_tree_node("wireframe", show_wireframe, true, "options='w=140';align=' '");
		add_member_control(this, "show", show_wireframe, "toggle", "w=52;shortcut='w'");
		if (show) {
			align("\a");
			add_member_control(this, "line width", line_width, "value_slider", "min=1;max=20;ticks=true;log=true");
			add_member_control(this, "line color", line_color);
			align("\b");
			end_tree_node(show_wireframe);
		}

		show = begin_tree_node("surface", show_surface, true, "options='w=140';align=' '");
		add_member_control(this, "show", show_surface, "toggle", "w=52;shortcut='s'");
		if (show) {
			align("\a");
			add_member_control(this, "cull mode", cull_mode, "dropdown", "enums='none,back,front'");
			add_member_control(this, "color mapping", color_mapping, "dropdown", "enums='none,front,back,front+back'");
			add_member_control(this, "surface color", surface_color);
			add_member_control(this, "illumination", illumination_mode, "dropdown", "enums='none,one sided,two sided'");
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
		return true;
	}
	void destruct(context& ctx)
	{
		ref_sphere_renderer(ctx, -1);
	}
	void init_frame(context& ctx)
	{
		if (have_new_mesh) {
			// auto-compute mesh normals if not available
			if (!M.has_normals())
				M.compute_vertex_normals();
			// [re-]compute mesh render info
			mesh_info.destruct(ctx);
			mesh_info.construct_vbos(ctx, M);
			// bind mesh attributes to standard surface shader program
			mesh_info.bind(ctx, ctx.ref_surface_shader_program(true));

			// ensure that materials are presented in gui
			post_recreate_gui();
			have_new_mesh = false;

			// focus view on new mesh
			clipped_view* view_ptr = dynamic_cast<clipped_view*>(find_view_as_node());
			if (view_ptr) {
				view_ptr->set_scene_extent(M.compute_box());
				view_ptr->set_default_view();
			}
		}
	}
	void draw(context& ctx)
	{
		if (show_vertices) {
			sphere_renderer& sr = ref_sphere_renderer(ctx);
			sr.set_render_style(sphere_style);
			sr.set_position_array(ctx, M.get_positions());
			if (M.has_colors())
				sr.set_color_array(ctx, *reinterpret_cast<const std::vector<rgb>*>(M.get_color_data_vector_ptr()));
			sr.validate_and_enable(ctx);
			glDrawArrays(GL_POINTS, 0, (GLsizei)M.get_nr_positions());
			sr.disable(ctx);
		}
		if (show_wireframe) {
			GLfloat old_line_width;
			glGetFloatv(GL_LINE_WIDTH, &old_line_width);
			glLineWidth(line_width);
				ctx.ref_default_shader_program().enable(ctx);
					ctx.set_color(line_color);
					mesh_info.draw_wireframe(ctx);
				ctx.ref_default_shader_program().disable(ctx);
			glLineWidth(old_line_width);
		}
		if (show_surface) {
			// ensure that opengl culling is identical to shader program based culling
			GLboolean is_culling = glIsEnabled(GL_CULL_FACE);
			GLint cull_face;
			glGetIntegerv(GL_CULL_FACE_MODE, &cull_face);

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
			prog.enable(ctx);
			ctx.set_color(surface_color);
			prog.disable(ctx);
			
			// render the mesh from the vertex buffers with selected program
			mesh_info.render_mesh(ctx, prog);
			
			// recover opengl culling mode
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
cgv::base::factory_registration<mesh> mesh_fac("new/media/mesh", 'M');
