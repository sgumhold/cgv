#include <cgv/defines/quote.h>
#include <cgv/base/node.h>
#include <cgv/render/context.h>
#include <cgv/render/drawable.h>
#include <cgv/render/shader_program.h>
#include <cgv_gl/box_renderer.h>
#include <cgv/gui/provider.h>

class cgv_proc_test : 
	public cgv::base::node,
	public cgv::render::drawable,
	public cgv::gui::provider
{
public:
	// render style for rendering vertices as spheres
	cgv::render::box_render_style brs;

	std::vector<box3> boxes;
	std::vector<rgb> box_colors;

	bool use_procedural_shader;
	int noise_type;
	cgv::render::shader_program noisy_box_prog;
	float noise_eps;
	float noise_zoom;
	float noise_color_scale;
	float noise_bump_scale;
	int nr_octaves;
	float lacunarity;
	float gain;
	float offset;

public:
	cgv_proc_test() : cgv::base::node("cgv_proc_test")
	{
		use_procedural_shader = true;
		noise_type = 3;
		noise_eps = 0.00001f;
		noise_zoom = 40.0f;
		noise_bump_scale = 0.1f;
		noise_color_scale = 0.05f;
		nr_octaves = 4;
		lacunarity = 2.0f;
		gain = 0.5f;
		offset = 0.9f;

		boxes.push_back(box3(vec3(0, 0, 0), vec3(1, 0.1f, 1)));
		boxes.push_back(box3(vec3(0.4f, 0.1f, 0.3f), vec3(0.6f, 1, 0.7f)));
		box_colors.push_back(rgb(0.5f, 0.5f, 0.5f));
		box_colors.push_back(rgb(1, 0.8f, 0));
	}
	std::string get_type_name() const
	{
		return "cgv_proc_test";
	}
	bool self_reflect(cgv::reflect::reflection_handler& rh)
	{
		return
			rh.reflect_member("use_procedural_shader", use_procedural_shader) &&
			rh.reflect_member("noise_type", noise_type) &&
			rh.reflect_member("noise_eps", noise_eps) &&
			rh.reflect_member("noise_zoom", noise_zoom) &&
			rh.reflect_member("noise_color_scale", noise_color_scale) &&
			rh.reflect_member("noise_bump_scale", noise_bump_scale) &&
			rh.reflect_member("nr_octaves", nr_octaves) &&
			rh.reflect_member("lacunarity", lacunarity) &&
			rh.reflect_member("gain", gain) &&
			rh.reflect_member("offset", offset);
	}
	void on_set(void* member_ptr)
	{
		update_member(member_ptr);
		post_redraw();
	}
	bool init(cgv::render::context& ctx)
	{
		cgv::render::ref_box_renderer(ctx, 1);
		if (!noisy_box_prog.build_program(ctx, "noisy_box.glpr", true)) {
			std::cerr << "could not bould noisy_box_prog" << std::endl;
			abort();
		}
		return true;
	}
	void clear(cgv::render::context& ctx)
	{
		cgv::render::ref_box_renderer(ctx, -1);
	}
	void draw(cgv::render::context& ctx)
	{
		// activate render styles
		auto& br = cgv::render::ref_box_renderer(ctx);
		br.set_render_style(brs);
		if (use_procedural_shader) {
			br.set_prog(noisy_box_prog);
			noisy_box_prog.set_uniform(ctx, "noise_type", (GLint)noise_type);
			noisy_box_prog.set_uniform(ctx, "eps", noise_eps);
			noisy_box_prog.set_uniform(ctx, "noise_zoom", noise_zoom);
			noisy_box_prog.set_uniform(ctx, "noise_color_scale", noise_color_scale);
			noisy_box_prog.set_uniform(ctx, "noise_bump_scale", noise_bump_scale);
			noisy_box_prog.set_uniform(ctx, "nr_octaves", nr_octaves);
			noisy_box_prog.set_uniform(ctx, "lacunarity", lacunarity);
			noisy_box_prog.set_uniform(ctx, "gain", gain);
			noisy_box_prog.set_uniform(ctx, "offset", offset);
		}
		br.set_box_array(ctx, boxes);
		br.set_color_array(ctx, box_colors);
		br.render(ctx, 0, boxes.size());
	}
	void create_gui()
	{
		add_decorator("cgv_noise_test", "heading", "level=2");
		add_member_control(this, "use_procedural_shader", use_procedural_shader, "check");
		add_member_control(this, "noise_type", (cgv::type::DummyEnum&)noise_type, "dropdown", "enums='generic,generic2,classic perlin,simplex'");
		add_member_control(this, "noise_eps", noise_eps, "value_slider", "min=0.00001;step=0.0000001;max=1;log=true;ticks=true");
		add_member_control(this, "noise_zoom", noise_zoom, "value_slider", "min=1;max=10000;log=true;ticks=true");
		add_member_control(this, "noise_color_scale", noise_color_scale, "value_slider", "min=0;max=10;log=true;ticks=true");
		add_member_control(this, "noise_bump_scale", noise_bump_scale, "value_slider", "min=0;max=10;log=true;ticks=true");
		add_member_control(this, "nr_octaves", nr_octaves, "value_slider", "min=1;max=24;ticks=true");
		add_member_control(this, "lacunarity", lacunarity, "value_slider", "min=0;max=10;log=true;ticks=true");
		add_member_control(this, "gain", gain, "value_slider", "min=0;max=2;ticks=true");
		add_member_control(this, "offset", offset, "value_slider", "min=0;max=1;ticks=true");
	}
};

#include <cgv/base/register.h>
cgv::base::object_registration<cgv_proc_test> cgv_prog_test_reg("cgv_prog_test");

#ifdef REGISTER_SHADER_FILES
#include <cgv_proc_test_shader_inc.h>
#endif

#ifdef CGV_FORCE_STATIC
extern cgv::base::registration_order_definition dro("stereo_view_interactor;cgv_proc_test");
#endif

