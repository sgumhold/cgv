#define _USE_MATH_DEFINES
#include <cmath>
#include <cgv/base/node.h>
#include <cgv/base/register.h>
#include <cgv/gui/event_handler.h>
#include <cgv/gui/provider.h>
#include <cgv/gui/key_event.h>
#include <cgv/gui/mouse_event.h>
#include <cgv/utils/file.h>
#include <cgv/utils/dir.h>
#include <cgv/render/drawable.h>
#include <cgv/render/attribute_array_binding.h>
#include <cgv_gl/gl/gl.h>
#include <cgv_gl/line_renderer.h>
#include <cgv/math/ftransform.h>
#include <cgv/media/image/image_writer.h>
#include <cgv/media/color_scale.h>
#include <libs/tt_gl_font/tt_gl_font.h>
// include self reflection helpers of used types (here vec3 & rgb)
#include <libs/cgv_reflect_types/math/fvec.h>
#include <libs/cgv_reflect_types/media/color.h>


class line_test :
	public cgv::base::node,
	public cgv::render::drawable,
	public cgv::gui::event_handler,
	public cgv::gui::provider
{
protected:
	std::vector<vec3> P;
	std::vector<rgba> C;
	std::vector<float> R;

	std::vector<GLuint> I;
	float angle[2];	
	bool smooth;
	bool strip;
	bool use_shader;
	cgv::render::vertex_buffer vbo;
	cgv::render::attribute_array_binding aab;
	cgv::render::line_render_style lrs;
	cgv::render::attribute_array_manager aam;

	bool vbo_out_of_date;
	void compute_positions()
	{
		P[0][0] = (float)cos(M_PI * angle[0]);
		P[0][1] = (float)sin(M_PI * angle[0]);
		P[2][0] = (float)cos(M_PI * angle[1]);
		P[2][1] = (float)sin(M_PI * angle[1]);
		vbo_out_of_date = true;
	}
public:
	line_test() : node("line test")
	{
		P.resize(3, vec3(0.0f));
		C.resize(3, rgba(1,1,1,1));
		C[1] = rgba(1, 0, 1, 0.5f);
		R.resize(3, 10.0f);
		R[1] = 50.0f;
		angle[0] = -0.5f;
		angle[1] = 0.5f;
		smooth = true;
		strip = true;
		I.resize(7);
		I[0] = 0;
		I[1] = 1;
		I[2] = 2;
		I[3] = 0;
		I[4] = 1;
		I[5] = 1;
		I[6] = 2;
		use_shader = true;
		lrs.blend_lines = true;
		lrs.screen_aligned = false;
		lrs.measure_line_width_in_pixel = false;
		lrs.halo_color = rgba(1.0f, 1.0f, 0.0f, 1.0f);
		lrs.blend_width_in_pixel = 1.0f;
		lrs.percentual_halo_width = 40.0f;
		compute_positions();
	}
	void on_set(void* member_ptr)
	{
		if ((member_ptr >= &P.front() && member_ptr < &P.front() + 3) ||
			(member_ptr >= &C.front() && member_ptr < &C.front() + 3) ||
			(member_ptr >= &R.front() && member_ptr < &R.front() + 3)) {
			vbo_out_of_date = true;
		}
		if (member_ptr == &angle[0] || member_ptr == &angle[1])
			compute_positions();

		update_member(member_ptr);
		post_redraw();
	}
	void stream_help(std::ostream& os)
	{
		os << "line test: " << std::endl;
	}
	void stream_stats(std::ostream& os)
	{
		os << "line test: " << angle[0] << ", " << angle[1] << std::endl;
	}
	bool handle(cgv::gui::event& e)
	{
		if (e.get_kind() == cgv::gui::EID_KEY) {
			cgv::gui::key_event& ke = reinterpret_cast<cgv::gui::key_event&>(e);
			if (ke.get_action() == cgv::gui::KA_RELEASE)
				return false;
			switch (ke.get_key()) {
			case 'S':
				smooth = !smooth;
				on_set(&smooth);
				break;
			case 'T':
				strip = !strip;
				on_set(&strip);
				break;
			}
		}
		return false;
	}
	std::string get_type_name() const { return "line_test"; }
	bool self_reflect(cgv::reflect::reflection_handler& rh)
	{
		return 
			rh.reflect_member("smooth", smooth) &&
			rh.reflect_member("strip", strip);
	}
	bool init(cgv::render::context& ctx)
	{
		aam.init(ctx);
		cgv::render::ref_line_renderer(ctx, 1);
		aab.create(ctx);
		vbo.create(ctx, 7 * sizeof(GLint) + 3 * sizeof(vec3) + 3 * sizeof(rgba) + 3 * sizeof(float));
		auto& prog = ctx.ref_default_shader_program();
		size_t offset = 0;
		aab.set_element_array(ctx, vbo);
		offset += 7 * sizeof(GLint);
		aab.set_attribute_array(ctx, prog.get_attribute_location(ctx, "position"),
			cgv::render::get_element_type(P.front()), vbo, offset, 3);
		offset += 3 * sizeof(vec3);
		aab.set_attribute_array(ctx, prog.get_attribute_location(ctx, "color"),
			cgv::render::get_element_type(C.front()), vbo, offset, 3);
		return true;
	}
	void update_vbo(cgv::render::context& ctx)
	{
		auto& prog = ctx.ref_default_shader_program();
		size_t offset = 0;
		vbo.replace(ctx, offset, I.data(), 7);
		offset += 7 * sizeof(GLint);
		vbo.replace(ctx, offset, P.data(), 3);
		offset += 3 * sizeof(vec3);
		vbo.replace(ctx, offset, C.data(), 3);
		offset += 3 * sizeof(rgba);
		vbo.replace(ctx, offset, R.data(), 3);
		offset += 3 * sizeof(float);
	}
	void clear(cgv::render::context& ctx)
	{
		aam.destruct(ctx);
		aab.destruct(ctx);
		cgv::render::ref_line_renderer(ctx, -1);
		vbo.destruct(ctx);
	}
	void draw(cgv::render::context& ctx)
	{
		glDisable(GL_CULL_FACE);
		if (use_shader) {
			auto& lr = cgv::render::ref_line_renderer(ctx, 0);
			lr.set_render_style(lrs);
			lr.enable_attribute_array_manager(ctx, aam);
			if (vbo_out_of_date) {
				update_vbo(ctx);
				lr.set_position_array(ctx, P);
				lr.set_color_array(ctx, C);
				lr.set_line_width_array(ctx, R);
				lr.set_indices(ctx, I);
				vbo_out_of_date = false;
			}
			lr.render(ctx, strip ? 0 : 3, strip ? 3 : 4, strip);
			lr.disable_attribute_array_manager(ctx, aam);
		}
		else {
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			auto& prog = ctx.ref_default_shader_program();
			ctx.set_color(C[1]);
			glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
			if (smooth)
				glEnable(GL_LINE_SMOOTH);
			else
				glDisable(GL_LINE_SMOOTH);
			glLineWidth(R[1]);
			aab.enable(ctx);
			prog.enable(ctx);
			if (strip)
				glDrawElements(GL_LINE_STRIP, 3, GL_UNSIGNED_INT, 0);
			else
				glDrawElements(GL_LINES, 4, GL_UNSIGNED_INT, reinterpret_cast<const void*>(3 * sizeof(GLint)));
			prog.disable(ctx);
			aab.disable(ctx);
			glDisable(GL_BLEND);
		}
	}
	void create_gui()
	{
		add_decorator("line test", "heading", "level=1");
		add_member_control(this, "use_shader", use_shader, "toggle");
		add_member_control(this, "smooth", smooth, "toggle");
		add_member_control(this, "strip", strip, "toggle");
		add_member_control(this, "a0", angle[0], "value_slider", "min=-3.14;max=3.14;ticks=true");
		add_member_control(this, "a1", angle[1], "value_slider", "min=-3.14;max=3.14;ticks=true");
		add_member_control(this, "z0", P[0][2], "value_slider", "min=-1;max=1;ticks=true");
		add_member_control(this, "z1", P[1][2], "value_slider", "min=-1;max=1;ticks=true");
		add_member_control(this, "z2", P[2][2], "value_slider", "min=-1;max=1;ticks=true");
		add_member_control(this, "C0", C[0]);
		add_member_control(this, "C1", C[1]);
		add_member_control(this, "C2", C[2]);
		add_member_control(this, "R0", R[0], "value_slider", "min=0;max=20;log=true;ticks=true");
		add_member_control(this, "R1", R[1], "value_slider", "min=0;max=20;log=true;ticks=true");
		add_member_control(this, "R2", R[2], "value_slider", "min=0;max=20;log=true;ticks=true");
		if (begin_tree_node("style", lrs)) {
			align("\a");
			add_gui("lrs", lrs);
			align("\b");
			end_tree_node(lrs);
		}
	}
};

#include <cgv/base/register.h>

cgv::base::object_registration<line_test> reg_line_test("");

#ifdef CGV_FORCE_STATIC
cgv::base::registration_order_definition ro_def("stereo_view_interactor;font_test");
#endif
