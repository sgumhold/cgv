#include <cgv/signal/rebind.h>
#include <cgv/base/register.h>
#include <cgv/gui/provider.h>
#include <cgv/render/drawable.h>
#include <cgv/render/frame_buffer.h>
#include <cgv/render/texture.h>
#include <cgv/utils/convert.h>
#include <cgv_gl/gl/gl.h>
#include <cgv_gl/gl/gl_transparent_renderer.h>
#include <cgv_gl/box_renderer.h>
#include <cgv_gl/gl/mesh_drawable.h>
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
	public cgv::render::gl::mesh_drawable,
	public provider
{
protected:
	gl_transparent_renderer transparent_renderer;
	// peeling configuration
	enum PeelingMode { PEEL_NTH_LAYER, ORDER_INDEPENDENT_TRANSPARENCY } peeling_mode;
	int max_nr_layers;
	float transparency;
	// depth peeler configuration
	bool front_to_back;
	bool two_sided;
	double epsilon;
	// simple scene
	std::vector<box3> boxes;
	std::vector<rgba> box_colors;
	box_render_style brs;
	// debug helpers
	bool write_images;
public:
	depth_peeler_test() 
	{
		max_nr_layers = 3;
		transparency = 0.5f;
		two_sided = false;
		front_to_back = true;
		peeling_mode = PEEL_NTH_LAYER; // ORDER_INDEPENDENT_TRANSPARENCY;
		epsilon = 0.00002;
		write_images = false;
		brs.map_color_to_material = cgv::render::CM_COLOR_AND_OPACITY;
		unsigned n = 20;
		float step = 1.0f / (n - 1);
		for (unsigned i = 0; i < n; ++i) {
			float v = i * step;
			boxes.push_back(box3(vec3(-1, -1, v), vec3(1, 1, v + 0.5f*step)));
			box_colors.push_back(cgv::media::color<float, cgv::media::HLS, cgv::media::OPACITY>(v, 0.5f, 1.0f, 0.5f));
		}
	}
	std::string get_type_name() const 
	{
		return "depth_peeler_test"; 
	}
	void on_set(void* member_ptr)
	{
		update_member(member_ptr);
		post_redraw();
	}
	bool init(context& ctx)
	{
		if (!transparent_renderer.init(ctx)) {
			std::cerr << "could not init transparent renderer" << std::endl;
			exit(0);
		}
		ref_box_renderer(ctx, 1);
		connect(transparent_renderer.render_callback, this, &depth_peeler_test::render_scene);
		ctx.set_bg_clr_idx(0);
		return cgv::render::gl::mesh_drawable::init(ctx);
	}
	void init_frame(context& ctx) 
	{
		transparent_renderer.set_depth_bias((float)epsilon);
		if (front_to_back)
			transparent_renderer.set_front_to_back();
		else
			transparent_renderer.set_back_to_front();
		transparent_renderer.init_frame(ctx);
		cgv::render::gl::mesh_drawable::init_frame(ctx);
	}
	void render_scene(context& ctx)
	{
		for (auto& c : box_colors)
			c[3] = 1.0f - transparency;
		auto& R = cgv::render::ref_box_renderer(ctx);
		R.set_attribute_array_manager(ctx, 0);
		R.set_render_style(brs);
		R.set_box_array(ctx, boxes);
		R.set_color_array(ctx, box_colors);
		R.render(ctx, 0, boxes.size());
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
		ref_box_renderer(ctx, -1);
	}
	
	void create_gui()
	{
		add_decorator("Peeling Configuration", "heading", "level=2");
		add_member_control(this, "peeling_mode", peeling_mode, "dropdown", "enums='show nth layer,order independent transparency'");
		add_member_control(this, "transparency", transparency, "value_slider", "min=0;max=1;ticks=true");
		add_member_control(this, "max_nr_layers", max_nr_layers, "value_slider", "min=0;max=22;ticks=true");

		if (begin_tree_node("Depth Peeler Configuration", peeling_mode, true, "level=3")) {
			align("\a");
				add_member_control(this, "front_to_back", front_to_back, "check");
				add_member_control(this, "two_sided", two_sided, "check");
				add_member_control(this, "epsilon", epsilon, "value_slider", "min=0.00001;max=0.1;step=0.00001;ticks=true;log=true");
			align("\b");
			end_tree_node(peeling_mode);
		}
		if (begin_tree_node("Debugging", write_images, true, "level=3")) {
			align("\a");
				add_member_control(this, "write_images", write_images, "toggle");
			align("\b");
			end_tree_node(peeling_mode);
		}
		if (begin_tree_node("Render Style", brs)) {
			align("\a");
			add_gui("box rs", brs);
			align("\b");
			end_tree_node(brs);
		}
	}
};

factory_registration<depth_peeler_test> fr_depth_peeler_test("depth_peeler_test", "shortcut='Ctrl-Shift-P';menu_text='new/render/depth peeler'", true);

