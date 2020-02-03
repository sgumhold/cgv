#include <cgv/signal/rebind.h>
#include <cgv/base/register.h>
#include <cgv/gui/provider.h>
#include <cgv/gui/trigger.h>
#include <cgv/render/drawable.h>
#include <cgv/render/texture.h>
#include <cgv/render/frame_buffer.h>
#include <cgv/render/shader_program.h>
#include <cgv_gl/gl/gl.h>
#include <cgv/media/font/font.h>
#include <cgv/math/ftransform.h>

using namespace cgv::base;
using namespace cgv::data;
using namespace cgv::media::font;
using namespace cgv::signal;
using namespace cgv::render;
using namespace cgv::gui;

class gpu_test : 
	public base,
	public drawable,
	public provider
{
protected:
	font_face_ptr font_face;

	texture tex;
	texture img_tex;
	texture depth_tex;
	render_buffer db;

	shader_program prog;
	shader_code vs, fs, v_lib, f_lib;

	frame_buffer fb;

	double angle;
	bool  show_tex, show_img_tex;
	bool do_shader_setup_by_hand;
	bool use_depth_texture;
public:
	/// define format and texture filters in constructor
	gpu_test() : 
		tex("uint8[R,G,B,A]", TF_NEAREST, TF_NEAREST),
		img_tex("uint8[R,G,B]", TF_NEAREST, TF_NEAREST),
		depth_tex("[D]", TF_NEAREST, TF_NEAREST),
		db("[D]")
	{
		font_face = find_font("Arial")->get_font_face(FFA_ITALIC);
		angle = 0;
		show_tex = show_img_tex = true;
		use_depth_texture = true;
		do_shader_setup_by_hand = false;
		connect(get_animation_trigger().shoot, this, &gpu_test::timer_event);
	}
	/// timer increments angle, updates gui and posts a redraw
	void timer_event(double t, double dt)
	{
		angle += 100*dt;
		update_member(&angle);
		post_redraw();
	}
	
	std::string get_type_name() const 
	{
		return "gpu_test"; 
	}
	
	bool init(context& ctx)
	{
		bool success = true;

		// create textures
		success = img_tex.create_from_image(ctx, "res://cgv_logo.png");

		// create in the size of the 3D view
		int w = ctx.get_width();
		int h = ctx.get_height();

		tex.set_width(w);
		tex.set_height(h);
		success = tex.create(ctx) && success;

		depth_tex.set_width(w);
		depth_tex.set_height(h);
		success = depth_tex.create(ctx) && success;

		// set up frame buffer object
		db.create(ctx,w,h);
		success = fb.create(ctx,w,h) && success;
		if (use_depth_texture)
			fb.attach(ctx,depth_tex);
		else
			fb.attach(ctx,db);

		fb.attach(ctx,tex);
		// and check whether it is complete now
		success = fb.is_complete(ctx) && success;

		if (do_shader_setup_by_hand) {
			// set up shader program by reading and compiling each code
			success = vs.read_and_compile(ctx, "gpu_test.glvs") && success;
			success = fs.read_and_compile(ctx, "gpu_test.glfs") && success;
			success = v_lib.read_and_compile(ctx, "my_support_functions.glsl", ST_VERTEX) && success;
			success = f_lib.read_and_compile(ctx, "my_support_functions.glsl", ST_FRAGMENT) && success;
			success = prog.create(ctx) && success;
			success = prog.attach_code(ctx, vs) && success;
			success = prog.attach_code(ctx, fs) && success;
			success = prog.attach_code(ctx, v_lib) && success;
			success = prog.attach_code(ctx, f_lib) && success;
			success = prog.link(ctx, true) && success;
		}
		else
			// set up shader program through shader program file
			success = prog.build_program(ctx, "gpu_test.glpr") && success;

		if (!success)
			std::cout << "init failed" << std::endl;
		return success;
	}
	void init_frame(context& ctx)
	{
		// activate frame buffer object rendering to texture tex
		fb.enable(ctx);
		glPushAttrib(GL_COLOR_BUFFER_BIT);
			// clear screen
			glClearColor(0,1,1,1);
			glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
			// draw text
			//ctx.ref_default_shader_program().enable(ctx);
				ctx.set_color(rgb(0.0f));
				ctx.push_pixel_coords();
					ctx.set_cursor(200,400);
					ctx.enable_font_face(font_face, 40);
					ctx.output_stream() << "test\ntext" << std::endl;
				ctx.pop_pixel_coords();
			//ctx.ref_default_shader_program().disable(ctx);

			// draw rotating cube
			ctx.ref_surface_shader_program().enable(ctx);
				ctx.set_color(rgb(1.0f,0.0f,0.0f));
				ctx.push_modelview_matrix();
					ctx.mul_modelview_matrix(cgv::math::rotate4<double>(angle, 1, 0, 0));
					ctx.tesselate_unit_cube();
				ctx.pop_modelview_matrix();
			ctx.ref_surface_shader_program().disable(ctx);
		glPopAttrib();
		fb.disable(ctx);
	}
	void draw(context& ctx)
	{
		// enable textures in different texture units
		tex.enable(ctx, 0);
		img_tex.enable(ctx, 1);
			// enable shader program
			prog.enable(ctx);
				ctx.set_color(rgb(1.0f));
				// set uniform variables including texture units
				prog.set_uniform(ctx,"show_tex", show_tex);
				prog.set_uniform(ctx,"show_img_tex", show_img_tex);
				prog.set_uniform(ctx,"tex", 0);
				prog.set_uniform(ctx,"img_tex", 1);
				prog.set_uniform(ctx, "map_color_to_material", 3);
				// draw scene
				ctx.tesselate_unit_cube();
			// disable shader program
			prog.disable(ctx);
			// and textures
		img_tex.disable(ctx);
		tex.disable(ctx);
	}
	void clear(context& ctx)
	{
		// destruct all allocated objects
		tex.destruct(ctx);
		img_tex.destruct(ctx);
		depth_tex.destruct(ctx);
		db.destruct(ctx);
		fb.destruct(ctx);
		prog.destruct(ctx);
		vs.destruct(ctx);
		fs.destruct(ctx);
		v_lib.destruct(ctx);
		f_lib.destruct(ctx);
	}
	void create_gui()
	{
		add_view("angle", angle);
		add_member_control(this, "show_tex", show_tex, "check");
		add_member_control(this, "show_img_tex", show_img_tex, "check");
	}
};

factory_registration<gpu_test> fr_gpu_test("new/render/gpu_test", 'U', true);