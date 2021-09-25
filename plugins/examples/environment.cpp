#include <cgv/base/node.h>
#include <cgv/gui/event_handler.h>
#include <cgv/gui/mouse_event.h>
#include <cgv/gui/provider.h>
#include <cgv/render/drawable.h>
#include <cgv_gl/gl/gl_context.h>
#include <cgv_glutil/box_render_data.h>
#include <cgv_glutil/shader_library.h>

using namespace cgv::render;

class environment_demo :
	public cgv::base::node,
	public cgv::render::drawable,
	public cgv::gui::provider,
	public cgv::gui::event_handler
{
protected:
	view* view_ptr;

	cgv::glutil::shader_library shaders;

	vec2 sun_position;

	cgv::glutil::box_render_data<> box_rd;
	box_render_style box_style;

public:
	environment_demo() : cgv::base::node("environment demo") {
		view_ptr = nullptr;
		shaders.add("sky", "sky.glpr");

		sun_position = vec2(0.0f, 0.75f);

		box_rd = cgv::glutil::box_render_data<>(true);
		box_style.surface_color = rgb(0.5f);
	}
	void stream_help(std::ostream& os) {
		return;
	}
	bool handle(cgv::gui::event& e) {
		return false;
	}
	void on_set(void* member_ptr) {
		
		//if(member_ptr == &foo) {
		//	for(size_t i = 0; i < texts.size(); ++i)
		//		texts.set_alignment(i, static_cast<cgv::render::TextAlignment>(text_align_h | text_align_v));
		//}

		post_redraw();
		update_member(member_ptr);
	}
	std::string get_type_name() const {
		return "environment_demo";
	}
	void clear(cgv::render::context& ctx) {
		ref_box_renderer(ctx, -1);

		shaders.clear(ctx);
	}
	bool init(cgv::render::context& ctx) {
		ref_box_renderer(ctx, 1);

		bool success = true;
		success &= shaders.load_shaders(ctx);
		if(box_rd.init(ctx)) {
			box_rd.add(vec3(0.0f, -0.1f, 0.0f), vec3(10.0f, 0.2f, 10.0f));
			box_rd.add(vec3(0.0f, 0.5f, 0.0f), vec3(1));
			box_rd.set_out_of_date();
		} else {
			success = false;
		}

		return success;
	}
	void init_frame(cgv::render::context& ctx) {
		if(!view_ptr) {
			if(view_ptr = find_view_as_node()) {}
		}
	}
	void draw(cgv::render::context& ctx) {
		if(!view_ptr)
			return;

		vec2 resolution(ctx.get_width(), ctx.get_height());
		auto& sky_prog = shaders.get("sky");
		sky_prog.enable(ctx);
		sky_prog.set_uniform(ctx, "resolution", resolution);
		sky_prog.set_uniform(ctx, "eye_pos", vec3(view_ptr->get_eye()));
		sky_prog.set_uniform(ctx, "sun_pos", sun_position);
		glDisable(GL_DEPTH_TEST);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glEnable(GL_DEPTH_TEST);
		sky_prog.disable(ctx);

		box_rd.render(ctx, ref_box_renderer(ctx), box_style);
	}
	void create_gui() {
		add_decorator("Environment Demo", "heading");

		add_decorator("Sun Position", "heading", "level=3");
		add_member_control(this, "Horizontal", sun_position[0], "value_slider", "min=0;max=1;step=0.01;ticks=true");
		add_member_control(this, "Vertical", sun_position[1], "value_slider", "min=0;max=1;step=0.01;ticks=true");
	}
};

#include <cgv/base/register.h>

/// register a factory to create new rounded cone texturing tests
cgv::base::factory_registration<environment_demo> environment_demo_fac("new/demo/environment_demo");
