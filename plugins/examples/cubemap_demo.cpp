#include <random>

#include <cgv/base/node.h>
#include <cgv/gui/event_handler.h>
#include <cgv/gui/mouse_event.h>
#include <cgv/gui/provider.h>
#include <cgv/media/image/image.h>
#include <cgv/media/image/image_reader.h>
#include <cgv/render/drawable.h>
#include <cgv/render/shader_library.h>
#include <cgv_gl/gl/gl_context.h>
#include <cgv_gl/gl/gl_tools.h>

using namespace cgv::render;

class cubemap_demo :
	public cgv::base::node,
	public cgv::render::drawable,
	public cgv::gui::provider,
	public cgv::gui::event_handler
{
protected:
	typedef cgv::math::fvec<signed char, 4u> cvec4;

	view* view_ptr;

	shader_library shaders;

	texture cubemap_tex;
	
public:
	cubemap_demo() : cgv::base::node("Cubemap Demo") {
		view_ptr = nullptr;
		shaders.add("cubemap", "cubemap.glpr");
	}
	void stream_help(std::ostream& os) {
		return;
	}
	bool handle(cgv::gui::event& e) {
		return false;
	}
	void on_set(void* member_ptr) {
		post_redraw();
		update_member(member_ptr);
	}
	std::string get_type_name() const {
		return "cubemap_demo";
	}
	void clear(cgv::render::context& ctx) {
		shaders.clear(ctx);
		cubemap_tex.destruct(ctx);
	}
	bool init(cgv::render::context& ctx) {
		bool success = true;
		success &= shaders.load_all(ctx);

		success &= cubemap_tex.create_from_images(ctx, "res://{right,left,bottom,top,front,back}.jpg", 0);

		return success;
	}
	void init_frame(cgv::render::context& ctx) {
		if(!view_ptr) {
			if((view_ptr = find_view_as_node())) {}
		}
	}
	void draw(cgv::render::context& ctx) {
		auto& cubemap_prog = shaders.get("cubemap");
		cubemap_prog.enable(ctx);
		cubemap_prog.set_uniform(ctx, "invert", true);
		cubemap_prog.set_uniform(ctx, "depth_value", 1.0f);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		cubemap_tex.enable(ctx, 0);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		cubemap_tex.disable(ctx);
		glDepthFunc(GL_LESS);
		cubemap_prog.disable(ctx);
	}
	void create_gui() {
		add_decorator("Cubemap Demo", "heading");
	}
};

#include <cgv/base/register.h>

/// register a factory to create new cubemap demos
cgv::base::factory_registration<cubemap_demo> cubemap_demo_fac("New/Demo/Cubemap Demo");
