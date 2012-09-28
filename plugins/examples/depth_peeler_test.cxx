#include <cgv/signal/rebind.h>
#include <cgv/base/register.h>
#include <cgv/gui/provider.h>
#include <cgv/render/drawable.h>
#include <cgv/render/frame_buffer.h>
#include <cgv/render/texture.h>
#include <cgv/utils/convert.h>
#include <cgv_gl/gl/gl.h>
#include <cgv_gl/gl/gl_transparent_renderer.h>
#include <cgv_gl/gl/gl_tools.h>

using namespace cgv::base;
using namespace cgv::signal;
using namespace cgv::data;
using namespace cgv::utils;
using namespace cgv::render;
using namespace cgv::render::gl;
using namespace cgv::gui;


class depth_peeler_test : 
	public base,
	public drawable,
	public provider
{
protected:
	gl_transparent_renderer transparent_renderer;
	// peeling configuration
	enum PeelingMode { PEEL_NTH_LAYER, ORDER_INDEPENDENT_TRANSPARENCY } peeling_mode;
	int max_nr_layers;
	double transparency;
	// depth peeler configuration
	bool front_to_back;
	bool two_sided;
	double epsilon;
	// debug helpers
	bool write_images;
	std::string file_name_prefix;
public:
	depth_peeler_test() 
	{
		max_nr_layers = 3;
		transparency = 0.5;
		two_sided = false;
		front_to_back = true;
		peeling_mode = ORDER_INDEPENDENT_TRANSPARENCY;
		epsilon = 0.00002;
		write_images = false;
		file_name_prefix = "e:/temp/dbg";
	}
	std::string get_type_name() const 
	{
		return "depth_peeler_test"; 
	}
	bool init(context& ctx)
	{
		if (!transparent_renderer.init(ctx)) {
			std::cerr << "could not init transparent renderer" << std::endl;
			exit(0);
		}
		connect(transparent_renderer.render_callback, this, &depth_peeler_test::render_scene);
		return true;
	}
	void init_frame(context& ctx) 
	{
		transparent_renderer.set_depth_bias((float)epsilon);
		if (front_to_back)
			transparent_renderer.set_front_to_back();
		else
			transparent_renderer.set_back_to_front();
		transparent_renderer.init_frame(ctx);
	}
	void render_scene(context& ctx)
	{
		glPushMatrix();
			glRotated(90,0,1,0);
			glScaled(0.2,0.2,0.2);
			glTranslated(-15.0,0,0);
			for (int i=0; i<=10; ++i) {
				int j = (43*i)%11;
				glColor4d((1-transparency)*0.07*j+0.3,0,(1-transparency)*(1-0.07*j)+0.3,transparency);
				ctx.tesselate_unit_cube();
				glTranslated(3.0,0,0);
			}
		glPopMatrix();
	}

	int peel_depth_layers(context& ctx)
	{

		// draw and use first layer (no depth peeling necessary)
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		render_scene(ctx);

		gl_depth_peeler& peeler = transparent_renderer;
		// repeat until no more fragments have been drawn
		int nr_fragments, nr_layers = 0;
		do {
			if (++nr_layers > max_nr_layers)
				break;
			// copy previous depth buffer to secondary depth buffer
			peeler.copy_depth_buffer(ctx);

			// then peel the next layer
			glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
			peeler.begin_layer(ctx);
			render_scene(ctx);
			nr_fragments = peeler.end_layer(ctx);
			//use_current_layer();
		}	while(nr_fragments > 0);
		return nr_layers;
	}

	void draw(context& ctx)
	{
		if (peeling_mode == PEEL_NTH_LAYER) {
			glPushAttrib(GL_ENABLE_BIT);
			if (two_sided)
				glDisable(GL_CULL_FACE);
			else
				glEnable(GL_CULL_FACE);

			peel_depth_layers(ctx);
			glPopAttrib();
		}
	}

	void finish_draw(context& ctx)
	{
		if (peeling_mode == ORDER_INDEPENDENT_TRANSPARENCY) {
			glPushAttrib(GL_ENABLE_BIT);
			if (two_sided)
				glDisable(GL_CULL_FACE);
			else
				glEnable(GL_CULL_FACE);
			transparent_renderer.render_transparent(ctx, max_nr_layers);
			glPopAttrib();
		}
	}

	void clear(context& ctx)
	{
		// destruct all allocated objects
		transparent_renderer.destruct(ctx);
	}
	
	void create_gui()
	{
		add_decorator("Peeling Configuration", "heading");
		connect_copy(
			add_control("peeling_mode", peeling_mode, "show nth layer,order independent transparency")->value_change,
							rebind(static_cast<drawable*>(this), 
							&drawable::post_redraw)
		);
		connect_copy(
			add_control("transparency", transparency, "value_slider", 
			            "min=0;max=1;ticks=true")->value_change,
							rebind(static_cast<drawable*>(this), 
							&drawable::post_redraw)
		);
		connect_copy(
			add_control("max_nr_layers", max_nr_layers, "value_slider", 
			            "min=0;max=22;ticks=true")->value_change,
							rebind(static_cast<drawable*>(this), 
							&drawable::post_redraw)
		);
		add_decorator("Depth Peeler Configuration", "heading");
		connect_copy(
			add_control("front_to_back", front_to_back, "check")->value_change,
							rebind(static_cast<drawable*>(this), 
							&drawable::post_redraw)
		);
		connect_copy(
			add_control("two_sided", two_sided, "check")->value_change,
							rebind(static_cast<drawable*>(this), 
							&drawable::post_redraw)
		);
		connect_copy(
			add_control("epsilon", epsilon, "value_slider", 
			            "min=0.00001;max=0.1;step=0.00001;ticks=true;log=true")->value_change,
							rebind(static_cast<drawable*>(this), 
							&drawable::post_redraw)
		);
		add_decorator("Debugging", "heading");
		connect_copy(
			add_control("write_images", write_images, "toggle")->value_change,
							rebind(static_cast<drawable*>(this), 
							&drawable::post_redraw)
		);
		add_control("file_name_prefix", file_name_prefix);
	}
};

factory_registration<depth_peeler_test> fr_depth_peeler_test("new/depth_peeler_test", 'D', true);