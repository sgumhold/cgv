#include <cgv/base/node.h>
#include <cgv/math/ftransform.h>
#include <cgv/gui/provider.h>
#include <cgv/render/drawable.h>
#include <cgv/render/shader_program.h>
#include <cgv_gl/gl/gl.h>

using namespace cgv::gui;
using namespace cgv::signal;
using namespace cgv::render;
using namespace cgv::base;

class buffer_test : public base, public provider, public drawable
{
public:
	enum TestType { TT_NONE, TT_STENCIL, TT_ALPHA, TT_ACCUM };
	TestType test_type;
	buffer_test()
	{
		test_type = TT_ACCUM;
	}
	std::string get_type_name() const 
	{
		return "buffer_test"; 
	}
	bool init(context& ctx)
	{
		gl::ensure_glew_initialized();
		ctx.attach_stencil_buffer();
		ctx.set_bg_stencil(0);
		ctx.set_bg_alpha(1);
		return true;
	}
	void small_square(context& ctx, const std::string& text)
	{
		ctx.push_projection_matrix();
			ctx.set_projection_matrix(cgv::math::identity4<double>());
			ctx.push_modelview_matrix();
				ctx.set_modelview_matrix(cgv::math::scale4<double>(0.25, 0.25, 0.25));
				cgv::render::shader_program& prog = ctx.ref_default_shader_program();
				prog.enable(ctx);
					ctx.set_color(cgv::rgb(1,0,0));
					ctx.tesselate_unit_square();
					//ctx.set_color(rgb(1, 1, 1));
					//ctx.enable_font_face(ctx.get_current_font_face(),20);
					//ctx.set_cursor(vec3(0,0,0).to_vec(), text, cgv::render::TA_BOTTOM);
					//ctx.output_stream() << text << std::endl;
	}
	void large_square(context& ctx, const std::string& text)
	{
					ctx.mul_modelview_matrix(cgv::math::scale4<double>(2,2,2));
					ctx.set_color(cgv::rgb(0, 1, 0));
					ctx.tesselate_unit_square();
					//ctx.set_color(rgb(0, 0, 0));
					//glColor3f(0, 0, 0);
					//ctx.enable_font_face(ctx.get_current_font_face(),20);
					//ctx.set_cursor(vec3(0,0,0).to_vec(), text, cgv::render::TA_BOTTOM);
					//ctx.output_stream() << text << std::endl;
				ctx.ref_default_shader_program().disable(ctx);
			ctx.pop_modelview_matrix();
		ctx.pop_projection_matrix();
	}
	void on_set(void* member_ptr)
	{
		post_redraw();
	}
	void draw(context& ctx)
	{
		if (test_type == TT_NONE)
			return;

		glPushAttrib(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);

		glDisable(GL_DEPTH_TEST);
		switch (test_type) {
		case TT_STENCIL : 
			glEnable(GL_STENCIL_TEST);
			glStencilFunc(GL_ALWAYS,0,255);
			glStencilOp(GL_INCR, GL_INCR, GL_INCR);
			small_square(ctx, "have stencil");
			glEnable(GL_STENCIL_TEST);
			glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
			glStencilFunc(GL_EQUAL,0,255);
			large_square(ctx, "no stencil");
			break;
		case TT_ALPHA :
			small_square(ctx, "have alpha");
			glEnable(GL_BLEND);
			glBlendFunc(GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA);
			large_square(ctx, "no alpha");
			break;
		case TT_ACCUM :
			small_square(ctx, "have accum");
			glReadBuffer(GL_BACK);
			glAccum(GL_LOAD, 1);
			glClear(GL_COLOR_BUFFER_BIT);
			large_square(ctx, "no accum");
			glAccum(GL_RETURN, 1);
			break;
		}
		glPopAttrib();
	}
	void create_gui()
	{
		add_decorator("buffer test", "heading");
		add_member_control(this, "test", test_type, "dropdown", "enums='NONE,STENCIL,ALPHA,ACCUM'");
	}
};
 
factory_registration<buffer_test> bt_fac("buffer_test", "shortcut='u';menu_text='New/Render/Buffer Test'", true);

