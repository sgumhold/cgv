#include <cgv/utils/file.h>
#include <cgv/base/node.h>
#include <cgv/gui/provider.h>
#include <cgv/render/drawable.h>
#include <cgv/render/shader_program.h>
#include <cgv/render/textured_material.h>
#include <cgv/media/mesh/obj_loader.h>
#include <cgv/media/axis_aligned_box.h>
#include <cgv/gui/provider.h>
#include <libs/cgv_gl/gl/gl.h>
#include <libs/cgv_gl/gl/gl_tools.h>
#include <libs/cgv_gl/gl/gl_mesh_drawable_base.h>
#include <algorithm>

class mesh_view : public cgv::base::node, public cgv::render::gl::gl_mesh_drawable_base, public cgv::gui::provider
{
public:
	typedef float crd_type;
protected:
	std::string file_name;
	bool wireframe;
//	std::vector<cgv::render::textured_material*> tex_materials;
public:
	mesh_view() : cgv::base::node("mesh")
	{
		wireframe = false;
	}
	std::string get_type_name() const
	{
		return "mesh_view";
	}
	bool self_reflect(cgv::reflect::reflection_handler& rh)
	{
		return rh.reflect_member("file_name", file_name) && rh.reflect_member("wireframe", wireframe);
	}
	void on_set(void* member_ptr)
	{
		if (member_ptr == &file_name) {
			if (read_mesh(file_name)) {
				set_name(cgv::utils::file::get_file_name(file_name));
				post_recreate_gui();

			}
		}
		update_member(member_ptr);
		post_redraw();
	}
	bool init(cgv::render::context& ctx)
	{
		return cgv::render::gl::ensure_glew_initialized();
	}
	void init_frame(cgv::render::context& ctx)
	{
		static bool my_tab_selected = false;
		if (!my_tab_selected) {
			my_tab_selected = true;
			cgv::gui::gui_group_ptr gg = ((provider*)this)->get_parent_group();
			if (gg) {
				cgv::gui::gui_group_ptr tab_group = gg->get_parent()->cast<cgv::gui::gui_group>();
				if (tab_group) {
					cgv::base::base_ptr c = gg;
					tab_group->select_child(c, true);
				}
			}
		}
	}

	void enable_material_color(const cgv::render::textured_material::color_type& c, float alpha, GLenum type) const
	{
		GLfloat v[4] = { c[0], c[1], c[2], c[3] * alpha };
		glMaterialfv(GL_FRONT_AND_BACK, type, v);
	}


	/// enable a material with textures
	void enable_material(cgv::render::context& ctx, cgv::render::shader_program& prog, const cgv::render::textured_material& mat, cgv::render::MaterialSide ms, float alpha)
	{
		bool do_alpha = (mat.get_diffuse_texture() != 0) && mat.get_diffuse_texture()->get_component_name(mat.get_diffuse_texture()->get_nr_components() - 1)[0] == 'A';
		GLuint flags = do_alpha ? GL_COLOR_BUFFER_BIT : GL_CURRENT_BIT;
		if (mat.get_bump_texture() != 0 || mat.get_diffuse_texture() != 0)
			flags |= GL_TEXTURE_BIT;
		flags |= GL_LIGHTING_BIT | GL_ENABLE_BIT;
		glPushAttrib(flags);

		glEnable(GL_LIGHTING);
		glDisable(GL_COLOR_MATERIAL);
		enable_material_color(mat.get_ambient(), alpha, GL_AMBIENT);
		enable_material_color(mat.get_diffuse(), alpha, GL_DIFFUSE);
		enable_material_color(mat.get_specular(), alpha, GL_SPECULAR);
		enable_material_color(mat.get_emission(), alpha, GL_EMISSION);
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, mat.get_shininess());

		bool use_bump_map = mat.get_bump_texture() != 0;
		if (use_bump_map)
			mat.get_bump_texture()->enable(ctx, 0);

		bool use_diffuse_map = mat.get_diffuse_texture() != 0;
		if (use_diffuse_map)
			mat.get_diffuse_texture()->enable(ctx, 1);

		prog.enable(ctx);
		prog.set_uniform(ctx, "use_bump_map", use_bump_map);
		if (use_bump_map) {
			prog.set_uniform(ctx, "bump_map", 0);
			prog.set_uniform(ctx, "bump_map_res", (int)(mat.get_bump_texture()->get_width()));
			prog.set_uniform(ctx, "bump_scale", 400 * mat.get_bump_scale());
		}
		prog.set_uniform(ctx, "use_diffuse_map", use_diffuse_map);
		if (use_diffuse_map)
			prog.set_uniform(ctx, "diffuse_map", 1);
		cgv::render::gl::set_lighting_parameters(ctx, prog);
		if (do_alpha) {
			glEnable(GL_ALPHA_TEST);
			switch (mat.get_alpha_test_func()) {
			case cgv::render::textured_material::AT_ALWAYS: glAlphaFunc(GL_ALWAYS, mat.get_alpha_threshold()); break;
			case cgv::render::textured_material::AT_LESS: glAlphaFunc(GL_LESS, mat.get_alpha_threshold()); break;
			case cgv::render::textured_material::AT_EQUAL: glAlphaFunc(GL_EQUAL, mat.get_alpha_threshold()); break;
			case cgv::render::textured_material::AT_GREATER: glAlphaFunc(GL_GREATER, mat.get_alpha_threshold()); break;
			}
		}
		else
			glColor4f(1, 1, 1, alpha);
	}
	/// disable phong material
	void disable_material(cgv::render::context& ctx, cgv::render::shader_program& prog, const cgv::render::textured_material& mat)
	{
		prog.disable(ctx);
		if (mat.get_diffuse_texture())
			mat.get_diffuse_texture()->disable(ctx);
		if (mat.get_bump_texture())
			mat.get_bump_texture()->disable(ctx);
		glPopAttrib();
	}

	void draw(cgv::render::context& ctx)
	{
		if (wireframe)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		cgv::render::gl::gl_mesh_drawable_base::draw(ctx);
		if (wireframe)
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	void create_gui()
	{
		add_gui("file_name", file_name, "file_name", "title='Open Mesh File';filter='mesh files (obj):*.obj'");
		add_member_control(this, "wireframe", wireframe, "check", "shortcut='W'");
		add_decorator("Materials", "heading");
		for (unsigned m = 0; m < materials.size(); ++m) {
			add_gui(materials[m].ref_name(), static_cast<cgv::media::illum::obj_material&>(materials[m]));
		}
	}
};

#include <cgv/base/register.h>

cgv::base::object_registration<mesh_view> mesh_view("register mesh view");