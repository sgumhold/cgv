#include <cgv/base/node.h>
#include <cgv/base/register.h>
#include <cgv/gui/provider.h>
#include <cgv/render/drawable.h>
#include <cgv/gui/dialog.h>
#include <random>

// renderer headers include self reflection helpers for render styles
#include <cgv_gl/box_renderer.h>
#include <cgv_gl/sphere_renderer.h>

// include self reflection helpers of used types (here vec3 & rgb)
#include <libs/cgv_reflect_types/math/fvec.h>
#include <libs/cgv_reflect_types/media/color.h>


class cfg_sample :
	public cgv::base::node,          /// derive from node to integrate into global tree structure and to store a name
	public cgv::render::drawable,    /// derive from drawable for drawing the cube
	public cgv::base::argument_handler,
	public cgv::gui::provider
{
protected:
	// declare some member variables
	std::string s;
	int   i;
	unsigned n;
	vec3  p;
	rgb   c;
	float r;
	std::default_random_engine e;

	cgv::render::sphere_render_style srs;
	cgv::render::box_render_style brs;
public:
	cfg_sample() : node("cfg_sample_instance")
	{
		s = "hello config";
		i = 5;
		n = 10;
		p = vec3(0.0f);
		c = rgb(1,0.3f,0.4f);
		r = 0.2f;
	}
	void handle_args(std::vector<std::string>& args)
	{
		s = "";
		for (auto a : args) {
			s = s.empty() ? a : s+";"+a;
		}
		// clear args vector as all args have been processed
		args.clear();
		// announce change in s
		on_set(&s);
	}
	std::string get_type_name() const { return "cfg_sample"; }
	bool self_reflect(cgv::reflect::reflection_handler& rh)
	{
		return
			rh.reflect_member("s", s) &&
			rh.reflect_member("i", i) &&
			rh.reflect_member("n", n) &&
			rh.reflect_member("p", p) &&
			rh.reflect_member("c", c) &&
			rh.reflect_member("r", r) &&
			rh.reflect_member("srs", srs) &&
			rh.reflect_member("brs", brs);
	}
	bool init(cgv::render::context& ctx)
	{
		auto& br = cgv::render::ref_box_renderer(ctx, 1);
		auto& sr = cgv::render::ref_sphere_renderer(ctx, 1);
		return br.ref_prog().is_linked() && sr.ref_prog().is_linked();
	}
	void clear(cgv::render::context& ctx)
	{
		cgv::render::ref_box_renderer(ctx, -1);
		cgv::render::ref_sphere_renderer(ctx, -1);
	}
	void draw(cgv::render::context& ctx)
	{
		auto& br = cgv::render::ref_box_renderer(ctx);
		br.set_render_style(brs);
		std::vector<box3> B;
		std::vector<rgb> C;
		float w = 0.75f / n;
		for (unsigned j = 0; j < n; ++j) {
			float lambda = float(j) / (n - 1);
			B.push_back(box3(vec3(lambda, 0, 0), vec3(lambda + w, 1, 1)));
			C.push_back(j == i ? rgb(1, 0, 0) : rgb(0.5f, 0.5f, 0.5f));
		}
		br.set_box_array(ctx, B);
		br.set_color_array(ctx, C);
		br.render(ctx, 0, n);

		auto& sr = cgv::render::ref_sphere_renderer(ctx);
		sr.set_render_style(srs);
		sr.set_position_array(ctx, &p, 1);
		sr.set_radius_array(ctx, &r, 1);
		sr.set_color_array(ctx, &c, 1);
		sr.render(ctx, 0, 1);

		ctx.set_cursor((p + vec3(0, 1.2f * r, 0)).to_vec(), s, cgv::render::TA_BOTTOM);
		ctx.set_color(rgb(1-c[0], 1-c[1], 1-c[2]));
		ctx.output_stream() << s;
		ctx.output_stream().flush();
	}
	void pose_query(bool hide)
	{
		if (cgv::gui::query("new value for s:", s, hide))
			on_set(&s);
	}
	void on_set(void* member_ptr)
	{
		if (member_ptr == &n) {
			auto c_ptr = find_control(i);
			if (c_ptr)
				c_ptr->set("max", n - 1);

			if (i >= int(n)) {
				std::uniform_int_distribution<int> d(0, 1);
				switch (cgv::gui::question("i went out of range, correct it?", "yes,no,random", 0)) {
				case 2 :
					if (d(e) == 0) {
						cgv::gui::message("random says \"don't adapt\".");
						break;
					}
				case 0 : 
					i = n - 1;
					update_member(&i);
				default:
					break;
				}
			}
		}
		// consistent update in ui of vector valued members needs some help:
		if (member_ptr == &p) {
			update_member(&p[1]);
			update_member(&p[2]);
		}
		if (member_ptr == &p[1] || member_ptr == &p[2])
			update_member(&p);
		if (member_ptr == &c) {
			update_member(&c[1]);
			update_member(&c[2]);
		}
		if (member_ptr == &c[1] || member_ptr == &c[2])
			update_member(&c);

		// default implementation for all members
		update_member(member_ptr);
		post_redraw();
	}
	void create_gui()
	{
		add_decorator("cfg_sample", "heading", "level=1");
		connect_copy(add_button("query")->click, cgv::signal::rebind(this, &cfg_sample::pose_query, cgv::signal::_c<bool>(false)));
		connect_copy(add_button("password")->click, cgv::signal::rebind(this, &cfg_sample::pose_query, cgv::signal::_c<bool>(true)));
		add_member_control(this, "s", s);
		add_member_control(this, "i", i, "value_slider", "min=0;max=9;ticks=true");
		add_member_control(this, "n", n, "value_slider", "min=1;max=1000;ticks=true;log=true");
		add_gui("p", p, "vector", "options='min=-1;max=1;ticks=true'");
		add_member_control(this, "c", c);
		add_member_control(this, "r", r, "value_slider", "min=0.01;max=10;ticks=true;log=true");
	}
};


#include <cgv/base/register.h>

cgv::base::object_registration<cfg_sample> reg_cfg_sample("");

#ifdef CGV_FORCE_STATIC
cgv::base::registration_order_definition ro_def("stereo_view_interactor;cfg_sample");
#endif
