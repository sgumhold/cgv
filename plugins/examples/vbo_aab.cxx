#include <limits>
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
#include <cgv/media/illum/surface_material.h>
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

// define maximum index as restart index
#define RESTART_INDEX std::numeric_limits<cgv::type::uint32_type>::max()

/*

This example illustrates how to construct vertex buffer objects containing vertex
attribute arrays as well as element indices for triangle strip and line strip
rendering of surface and wireframe.

The typical workflow is

PREPARATION (implemented in init or init_frame method of drawable)
- compute vertex attribute arrays and element indices in std::vector<T> 
  containers on the GPU. In this example attributes are stored in an 
  interleaved fashion, implemented with the help of a vertex_type struct. 
- create vertex buffers from std::vector<T> containers.
  Careful with the element indices. These have to be stored in a separate
  vertex buffer that is marked to be of type VBT_INDICES.
- choose a shader program. Here the default shader programs provided by
  the context are used.
- bind array pointers at location indices queried from program to vertex buffers
  in an attribute_array_binding struct

RENDERING PHASE (implemented in draw and finish_draw method)
- the following steps should be done for the opaque parts in the draw() and
  for the transparent parts in the finish_draw() method
- enable shader program and configure its uniforms
- enable atribute array binding
- draw elements
- disable atribute array binding
- disable shader program

This example illustrates furthermore
- how to use glPrimitiveRestartIndex()/glEnable(GL_PRIMITIVE_RESTART) to
  render several strips with one draw call
- how to set attribute arrays of an attribute array manager of a renderer 
  to vertex buffer objects. This is done for vertex rendering with a sphere
  shader. The general approach is:

  PREPARATION

  - create your own attribute_array_manager in drawable::init method
  - choose renderer and set attribute_array_manager pointer of renderer
    to your attribute array manager
  - set attributes in renderer which automatically sets them in your 
    attribute array manager
  - set attribute_array_manager pointer of renderer to nullptr

  RENDERING

  - set attribute_array_manager pointer of renderer
    to your attribute array manager
  - enable renderer
  - draw primitives
  - disable renderer
  - set attribute_array_manager pointer of renderer to nullptr
*/
class vbo_aab : public node, public drawable, public provider
{
protected:
	bool generate_new_geometry;
	size_t nr_positions;
	size_t nr_triangle_elements, nr_line_elements;
	/// one vbo for all vertex attributes stored interleaved
	cgv::render::vertex_buffer vbo_attribs;
	/// one vbo for element indices used for triangle strip rendering
	cgv::render::vertex_buffer vbo_elements;
	///
	cgv::render::attribute_array_manager aam_sphere;
	///
	cgv::render::attribute_array_binding aab_surface;
	///
	cgv::render::attribute_array_binding aab_wireframe;
public:
	bool show_surface;
	CullingMode cull_mode;
	ColorMapping color_mapping;
	cgv::rgb  surface_color;
	IlluminationMode illumination_mode;
	surface_material material;

	bool show_wireframe;
	float line_width;
	bool map_color_to_wireframe;
	cgv::rgb  line_color;

	bool show_vertices;
	cgv::render::sphere_render_style sphere_style;

	// mesh generation parameters
	int n, m;
	float a, b;
	float lb, ub;
public:
	vbo_aab() : node("vbo_aab")
	{
		show_surface = true;
		cull_mode = CM_BACKFACE;
		color_mapping = cgv::render::CM_COLOR;
		surface_color = cgv::rgb(0.7f, 0.2f, 1.0f);
		illumination_mode = IM_ONE_SIDED;

		show_wireframe = true;
		line_width = 2.0f;
		line_color = cgv::rgb(0.6f,0.5f,0.4f);
		map_color_to_wireframe = false;

		show_vertices = true;
		a = 1;
		b = 0.2f;
		lb = 0.01f;
		ub = 2.0f;
		n = m = 20;

		generate_new_geometry = true;
	}
	struct vertex_type
	{
		cgv::vec3 position;
		cgv::vec3 normal;
		cgv::rgb  color;
	};
	cgv::box3 generate_dini_surface_geometry(cgv::render::context& ctx)
	{
		cgv::box3 box;
		box.invalidate();

		std::vector<vertex_type> V((n + 1)*m);
		std::vector<cgv::type::uint32_type> indices;

		// allocate per vertex colors of type rgb with float components
		int i;
		for (i = 0; i <= n; ++i) {
			float y = (float)i / n;
			float v = (ub - lb)*y + lb;
			if (i > 1)
				indices.push_back(RESTART_INDEX);
			for (int j = 0; j < m; ++j) {
				float x = (float)j / m;
				float u = float(4.0f*M_PI)*x;
				// add new position to the mesh (function returns position index, which is i*m+j in our case)

				int vi = i * m + j;
				vertex_type& vertex = V[vi];
				vertex.position = cgv::vec3(a*cos(u)*sin(v), a*sin(u)*sin(v), a*(cos(v) + log(tan(0.5f*v))) + b * u);

				float t1 = cos(v);
				float t2 = a * (-pow(t1, 0.2e1f) + 0.1e1f);
				float t3 = cos(u);
				float t4 = sin(u);
				float t5 = v / 0.2e1f;
				float t6 = sin(t5);
				t5 = cos(t5);
				float t7 = sin(v);
				float t8 = a / 0.2e1f;
				float t9 = 0.1e1f / t5;
				float t10 = 0.1e1f / t6;
				vertex.normal = cgv::vec3(
					a * (t8 * t3 * t7 - (b * t4 * t1 + t2 * t3) * t6 * t5) * t9 * t10,
					(t8 * t4 * t7 - (-b * t3 * t1 + t2 * t4) * t6 * t5) * a * t9 * t10,
					-t7 * t1 * a * a);
				vertex.normal.normalize();

				/* 
				// uncomment to generate cylinder 
				vertex.position = vec3(cos(0.5f*u), sin(0.5f*u), y);
				vertex.normal = vec3(cos(0.5f*u), sin(0.5f*u), 0);
				*/
				
				// set color
				vertex.color = cgv::rgb(x, y, 0.5f);

				// update bounding box
				box.add_point(vertex.position);

				// add two vertex indices to triangle strips
				if (i > 0) {
					indices.push_back(vi);
					indices.push_back(vi - m);
				}
			}
			if (i > 0) {
				indices.push_back(i*m);
				indices.push_back((i-1)*m);
			}
		}
		nr_triangle_elements = indices.size();

		// generate elements for wireframe rendering
		for (i = 0; i <= n; ++i) {
			if (i > 0)
				indices.push_back(RESTART_INDEX);
			for (int j = 0; j < m; ++j) 
				indices.push_back(i*m + j);
			indices.push_back(i*m);
		}
		for (i = 0; i < m; ++i) {
			indices.push_back(RESTART_INDEX);
			for (int j = 0; j <= n; ++j)
				indices.push_back(j*m + i);
		}
		nr_line_elements = indices.size() - nr_triangle_elements;
		construct_vbo_and_aab(ctx, V, indices);
		return box;
	}
	void construct_vbo_and_aab(
		cgv::render::context& ctx, 
		const std::vector<vertex_type>& V, 
		std::vector<cgv::type::uint32_type>& indices)
	{
		// destruct previously allocated vertex buffers
		vbo_attribs.destruct(ctx);
		vbo_elements.destruct(ctx);
		nr_positions = V.size();

		// transfer data into vertex buffers
		vbo_attribs.create(ctx, V);
		vbo_elements.type = cgv::render::VBT_INDICES; // mark buffer as index buffer
		vbo_elements.create(ctx, indices);

		// configure the attribute manager
		auto& sr = ref_sphere_renderer(ctx);
		sr.enable_attribute_array_manager(ctx, aam_sphere);
			sr.set_position_array(ctx, get_element_type(V.front().position), 
				vbo_attribs, 0, V.size(), sizeof(vertex_type));
			sr.set_color_array(ctx, get_element_type(V.front().color),
				vbo_attribs, 2*sizeof(cgv::vec3), V.size(), sizeof(vertex_type));
		sr.disable_attribute_array_manager(ctx, aam_sphere);

		// configure attribute array binding for surface rendering
		auto& pr = ctx.ref_surface_shader_program(false);
		aab_surface.bind_attribute_array(ctx, pr, "position",
			get_element_type(V.front().position), vbo_attribs, 
			0, nr_positions, sizeof(vertex_type));
		aab_surface.bind_attribute_array(ctx, pr, "normal",
			get_element_type(V.front().normal), vbo_attribs, 
			offsetof(vertex_type, normal), nr_positions, sizeof(vertex_type));
		aab_surface.bind_attribute_array(ctx, pr, "color",
			get_element_type(V.front().color), vbo_attribs, 
			offsetof(vertex_type, color), nr_positions, sizeof(vertex_type));
		aab_surface.set_element_array(ctx, vbo_elements);

		// configure attribute array binding for wireframe rendering
		auto& prw = ctx.ref_default_shader_program(false);
		aab_wireframe.bind_attribute_array(ctx, prw, "position",
			get_element_type(V.front().position), vbo_attribs, 
			0, nr_positions, sizeof(vertex_type));
		aab_wireframe.bind_attribute_array(ctx, prw, "color",
			get_element_type(V.front().color), vbo_attribs, 
			offsetof(vertex_type, color), nr_positions, sizeof(vertex_type));
		aab_wireframe.set_element_array(ctx, vbo_elements);
	}
	void on_set(void* member_ptr)
	{
		if (member_ptr == &n || member_ptr == &m ||
			member_ptr == &lb || member_ptr == &ub ||
			member_ptr == &a || member_ptr == &b) {

			generate_new_geometry = true;
		}
		update_member(member_ptr);
		post_redraw();
	}
	void create_gui()
	{
		add_decorator("vbo_aab", "heading", "level=2");
		if (begin_tree_node("generate", a, true)) {
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
		bool show = begin_tree_node("vertex spheres", show_vertices, false, "options='w=140';align=' '");
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
			add_member_control(this, "map_color_to_wireframe", map_color_to_wireframe, "check");
			align("\b");
			end_tree_node(show_wireframe);
		}

		show = begin_tree_node("surface", show_surface, true, "options='w=140';align=' '");
		add_member_control(this, "show", show_surface, "toggle", "w=52;shortcut='s'");
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
			if (begin_tree_node("material", material)) {
				align("\a");
				add_gui("material", material);
				align("\b");
				end_tree_node(material);
			}
			align("\b");
			end_tree_node(show_surface);
		}
	}
	bool init(context& ctx)
	{
		aam_sphere.init(ctx);
		aab_surface.create(ctx);
		aab_wireframe.create(ctx);
		ref_sphere_renderer(ctx, 1);
		return true;
	}
	void destruct(context& ctx)
	{
		vbo_attribs.destruct(ctx);
		vbo_elements.destruct(ctx);
		aab_surface.destruct(ctx);
		aab_wireframe.destruct(ctx);
		aam_sphere.destruct(ctx);
		ref_sphere_renderer(ctx, -1);
	}
	void init_frame(context& ctx)
	{
		if (generate_new_geometry) {
			cgv::box3 box = generate_dini_surface_geometry(ctx);
			generate_new_geometry = false;

			sphere_style.radius = float(0.05*sqrt(box.get_extent().sqr_length() / nr_positions));

			// focus view on new mesh
			clipped_view* view_ptr = dynamic_cast<clipped_view*>(find_view_as_node());
			if (view_ptr) {
				view_ptr->set_scene_extent(box);
				view_ptr->set_focus(box.get_center());
				view_ptr->set_y_extent_at_focus(box.get_extent().length());
			}
		}
	}
	void draw_surface(context& ctx)
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

		// choose a shader program
		shader_program& prog = ctx.ref_surface_shader_program(false);

		// enable program and configure it based on current settings
		prog.enable(ctx);
		prog.set_uniform(ctx, "culling_mode", (int)cull_mode); // fragment culling mode
		prog.set_uniform(ctx, "map_color_to_material", (int)color_mapping); // fragment color mapping mode
		prog.set_uniform(ctx, "illumination_mode", (int)illumination_mode); // fragment illumination mode

		// the set_uniform method only supports basic types and no structs like the material, where you would have
		// to set each member individually. Therefore the context provides a method that sets all material parameters
		// of the currently enabled program as used here
		ctx.set_material(material);
		// set default surface color for color mapping which only affects 
		// rendering if mesh does not have per vertex colors and color_mapping is on
		prog.set_attribute(ctx, prog.get_color_index(), surface_color);

		aab_surface.enable(ctx);
		glDrawElements(GL_TRIANGLE_STRIP, (GLsizei) nr_triangle_elements, GL_UNSIGNED_INT, 0);
		aab_surface.disable(ctx);

		prog.disable(ctx);

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
			sr.enable_attribute_array_manager(ctx, aam_sphere);
			sr.set_render_style(sphere_style);
			sr.render(ctx, 0, nr_positions);
			sr.disable_attribute_array_manager(ctx, aam_sphere);
		}

		// remember restart setting
		GLboolean is_restarting = glIsEnabled(GL_PRIMITIVE_RESTART);
		GLint restart_index;
		glGetIntegerv(GL_PRIMITIVE_RESTART_INDEX, &restart_index);

		// enable my own restart index
		glPrimitiveRestartIndex(RESTART_INDEX);
		glEnable(GL_PRIMITIVE_RESTART);

		if (show_wireframe) {
			// remember current line width
			GLfloat old_line_width;
			glGetFloatv(GL_LINE_WIDTH, &old_line_width);
			glLineWidth(line_width);

			// choose default shader program
			cgv::render::shader_program& prog = ctx.ref_default_shader_program();

			// enable aab and program (order is not important and could be swapped)
			aab_wireframe.enable(ctx);
			prog.enable(ctx);

				// in case of color mapping, use color attribute array
				if (map_color_to_wireframe)
					aab_wireframe.enable_array(ctx, prog.get_color_index());
				// otherwise turn color attribute array off and 
				// use set_color method of context to set vertex attribute to single value line_color
				else {
					aab_wireframe.disable_array(ctx, prog.get_color_index());
					ctx.set_color(line_color);
				}
				// draw all line strips with one draw call where the last pointer casted argument 
				// is the offset in the element buffer, where the line strips begin
				glDrawElements(GL_LINE_STRIP, (GLsizei) nr_line_elements, GL_UNSIGNED_INT,
					(void*)(sizeof(cgv::type::uint32_type)*nr_triangle_elements));

			// disable program and aab - again order is unimportant
			prog.disable(ctx);
			aab_wireframe.disable(ctx);

			// recover old line width
			glLineWidth(old_line_width);
		}
		// render opaque surfaces here without blending
		if (show_surface && material.get_transparency() < 0.01f)
			draw_surface(ctx);
		// recover restart settings
		glPrimitiveRestartIndex(restart_index);
		if (!is_restarting)
			glDisable(GL_PRIMITIVE_RESTART);
	}
	void finish_draw(context& ctx)
	{
		// remember restart setting
		GLboolean is_restarting = glIsEnabled(GL_PRIMITIVE_RESTART);
		GLint restart_index;
		glGetIntegerv(GL_PRIMITIVE_RESTART_INDEX, &restart_index);

		// enable my own restart index
		glPrimitiveRestartIndex(RESTART_INDEX);
		glEnable(GL_PRIMITIVE_RESTART);

		// render surfaces with transparency here with blending enabled
		if (show_surface && !(ctx.get_current_material()->get_transparency() < 0.01f)) {				
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			draw_surface(ctx);
			glDisable(GL_BLEND);
		}
		// recover restart settings
		glPrimitiveRestartIndex(restart_index);
		if (!is_restarting)
			glDisable(GL_PRIMITIVE_RESTART);

	}
};

#include <cgv/base/register.h>

/// register a factory to create new cubes
cgv::base::factory_registration<vbo_aab> vbo_aab_fac("New/Media/VBO AAB", 'V');
