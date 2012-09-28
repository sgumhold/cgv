#include <cgv/base/node.h>
#include <cgv/gui/provider.h>
#include <cgv/render/drawable.h>
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
	void init_frame(context& ctx)
	{
	}
	void small_square(context& ctx, const std::string& text)
	{
		glPushMatrix();
		glLoadIdentity();
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		glMatrixMode(GL_MODELVIEW);
		glScaled(0.25,0.25,0.25);
		glColor4f(1,0,0,0);
		ctx.tesselate_unit_square();
		glColor4f(1,1,1,0);
		ctx.enable_font_face(ctx.get_current_font_face(),20);
		ctx.set_cursor(cgv::math::vec<float>(0,0,0), text, cgv::render::TA_BOTTOM);
		ctx.output_stream() << text << std::endl;
	}
	void large_square(context& ctx, const std::string& text)
	{
		glScaled(2,2,2);
		glColor4f(0,1,0,1);
		ctx.tesselate_unit_square();
		glColor4f(0,0,0,1);
		ctx.enable_font_face(ctx.get_current_font_face(),20);
		ctx.set_cursor(cgv::math::vec<float>(0,0,0), text, cgv::render::TA_BOTTOM);
		ctx.output_stream() << text << std::endl;
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
	}
	void draw(context& ctx)
	{
		if (test_type == TT_NONE)
			return;

		glPushAttrib(GL_LIGHTING_BIT|GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
		glDisable(GL_LIGHTING);
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
		connect_copy(add_control("test", test_type, "NONE,STENCIL,ALPHA,ACCUM")->value_change, rebind(static_cast<drawable*>(this), &drawable::post_redraw));
	}
};
 

factory_registration<buffer_test> fr_buffer_test("new/buffer_test", 'Y', true);