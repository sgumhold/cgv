#include <cgv/base/node.h>
#include <cgv/signal/rebind.h>
#include <cgv/base/register.h>
#include <cgv/gui/provider.h>
#include <cgv/gui/trigger.h>
#include <cgv/gui/file_dialog.h>
#include <cgv/utils/file.h>
#include <cgv/utils/scan.h>
#include <cgv/utils/tokenizer.h>
#include <cgv/render/drawable.h>
#include <cgv_gl/box_renderer.h>
#include <cgv_gl/gl/gl.h>
#include <string>
#include <fstream>
#include <cgv/utils/stopwatch.h>

class vr_test : 
	public cgv::base::node,
	public cgv::render::drawable,
	public cgv::gui::provider
{
private:
	cgv::render::box_renderer renderer;
protected:
	std::vector<box3> boxes;
	std::vector<rgb> box_colors;

	cgv::render::surface_render_style style;

	void build_scene(
		float w, float d, float h, float W,
		float tw, float td, float th, float tW)
	{
		// construct floor
		boxes.push_back(box3(vec3(-0.5f*w, -W, -0.5f*d), vec3(0.5f*w, 0, 0.5f*d)));
		box_colors.push_back(rgb(0.5f, 0.5f, 0.5f));

		// construct walls
		boxes.push_back(box3(vec3(-0.5f*w, -W, -0.5f*d - W), vec3(0.5f*w, h, -0.5f*d)));
		box_colors.push_back(rgb(0.8f, 0.5f, 0.5f));
		boxes.push_back(box3(vec3(-0.5f*w, -W, 0.5f*d), vec3(0.5f*w, h, 0.5f*d + W)));
		box_colors.push_back(rgb(0.8f, 0.5f, 0.5f));

		boxes.push_back(box3(vec3(0.5f*w, -W, -0.5f*d - W), vec3(0.5f*w+W, h, 0.5f*d+W)));
		box_colors.push_back(rgb(0.5f, 0.8f, 0.5f));

		// construct ceiling
		boxes.push_back(box3(vec3(-0.5f*w-W, h, -0.5f*d-W), vec3(0.5f*w+W, h+W, 0.5f*d+W)));
		box_colors.push_back(rgb(0.5f, 0.5f, 0.8f));

		// construct table
		boxes.push_back(box3(vec3(-0.5f*tw - tW, th, -0.5f*td - tW), vec3(0.5f*tw + tW, th + tW, 0.5f*td + tW)));
		box_colors.push_back(rgb(0.5f, 0.4f, 0.0f));

		boxes.push_back(box3(vec3(-0.5f*tw, 0, -0.5f*td), vec3(-0.5f*tw + tW, th, -0.5f*td + tW)));
		boxes.push_back(box3(vec3(-0.5f*tw, 0,  0.5f*td), vec3(-0.5f*tw + tW, th,  0.5f*td + tW)));
		boxes.push_back(box3(vec3( 0.5f*tw, 0, -0.5f*td), vec3( 0.5f*tw + tW, th, -0.5f*td + tW)));
		boxes.push_back(box3(vec3( 0.5f*tw, 0,  0.5f*td), vec3( 0.5f*tw + tW, th,  0.5f*td + tW)));
		box_colors.push_back(rgb(0.5f, 0.4f, 0.0f));
		box_colors.push_back(rgb(0.5f, 0.4f, 0.0f));
		box_colors.push_back(rgb(0.5f, 0.4f, 0.0f));
		box_colors.push_back(rgb(0.5f, 0.4f, 0.0f));

		// construct chair
	}
public:
	vr_test() 
	{
		set_name("vr_test");
		build_scene(5,7,3,0.2f, 1.6f, 0.8f, 1.2f, 0.03f);
	}
	std::string get_type_name() const 
	{
		return "vr_test"; 
	}
	void create_gui()
	{
		add_decorator("vr_test", "heading", "level=2");
		add_gui("boxes", style);
	}

	bool init(cgv::render::context& ctx)
	{
		return renderer.init(ctx);
	}
	void draw(cgv::render::context& ctx)
	{
		renderer.set_render_style(style);
		renderer.set_box_array(ctx, boxes);
		renderer.set_color_array(ctx, box_colors);
		if (renderer.validate_and_enable(ctx)) {
			glDrawArrays(GL_POINTS, 0, boxes.size());
		}
		renderer.disable(ctx);
	}
};

#include <cgv/base/register.h>

cgv::base::object_registration<vr_test> vr_test_reg("");
