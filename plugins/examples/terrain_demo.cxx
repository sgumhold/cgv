#include <cgv/render/drawable.h>
#include <cgv/render/shader_program.h>
#include <cgv_gl/gl/gl.h>
#include <cgv/base/register.h>
#include <cgv/gui/trigger.h>
#include <cgv/gui/event_handler.h>
#include <cgv/gui/key_event.h>
#include <cgv/utils/ostream_printf.h>
#include <cgv/gui/provider.h>
#include <cgv/math/ftransform.h>
#include <cgv_proc/terrain_renderer.h>

using namespace cgv::base;
using namespace cgv::reflect;
using namespace cgv::gui;
using namespace cgv::signal;
using namespace cgv::render;

class terrain_demo : public base,		   // base class of all to be registered classes
					 public provider,	   // is derived from tacker, which is not necessary as base anymore
					 public event_handler, // necessary to receive events
					 public drawable	   // registers for drawing with opengl
{
  private:
	/// flag used to represent the state of the extensible gui node
	bool toggle;

  protected:
	std::vector<cgv::vec2> custom_positions;
	std::vector<unsigned int> custom_indices;
	terrain_render_style terrain_style;
	int grid_width = 10;
	int grid_height = 10;

	cgv::math::fvec<double, 3> translation = {0, -40, 0};

  public:
	/// initialize rotation angle
	terrain_demo() : toggle(false)
	{
		terrain_style.noise_layers.emplace_back(450.0F, 20.0F);
		terrain_style.noise_layers.emplace_back(300.0F, 15.0F);
		terrain_style.noise_layers.emplace_back(200.0F, 10.0F);
		terrain_style.noise_layers.emplace_back(150.0F, 7.5F);
		terrain_style.noise_layers.emplace_back(100.0F, 5.0F);
		terrain_style.noise_layers.emplace_back(80.0F, 4.0F);
		terrain_style.noise_layers.emplace_back(30.0F, 2.0F);
		terrain_style.noise_layers.emplace_back(7.5F, 0.75F);

		terrain_style.material.set_brdf_type(cgv::media::illum::BT_OREN_NAYAR);
		terrain_style.material.set_roughness(1.0f);
		terrain_style.material.set_ambient_occlusion(0.5f);
	}

	bool handle(event& e) override
	{
		return false;
	}

	void on_set(void* member_ptr) override
	{
		update_member(member_ptr);
		post_redraw();
	}

	/// return the type name of the class derived from base
	std::string get_type_name() const override
	{
		return "terrain_demo";
	}

	/// show help information
	void stream_help(std::ostream& os) override
	{
		os << "simple_cube:\a\n"
		   << "   change recursion depth: <up/down arrow>\n"
		   << "   change speed: <left/right arrow>\n"
		   << "   reset angle:  <space>\b\n";
	}

	bool init(context& ctx) override
	{
		cgv::render::ref_terrain_renderer(ctx, 1);
		terrain_style.load_default_textures(ctx);
		return true;
	}

	void generate_mesh()
	{
		custom_positions.clear();
		custom_indices.clear();

		std::vector<float> quadVertices = {
			  0.0f, 0.0f, //
			  1.0f, 0.0f, //
			  1.0f, 1.0f, //
			  0.0f, 1.0f, //
		};

		for (int row = 0; row < grid_height; row++) {
			for (int col = 0; col < grid_width; col++) {
				for (int i = 0; i < static_cast<int64_t>(quadVertices.size() / 2); i++) {
					float x = (quadVertices[i * 2] + static_cast<float>(row- grid_height/2)) * 100;
					float y = (quadVertices[i * 2 + 1] + static_cast<float>(col-grid_width/2)) * 100;
					custom_positions.emplace_back(x, y);
				}
			}
		}

		for (int row = 0; row < grid_height; row++) {
			for (int col = 0; col < grid_width; col++) {
				const int i = (row * grid_width + col) * 4;
				custom_indices.emplace_back(i);
				custom_indices.emplace_back(i + 1);
				custom_indices.emplace_back(i + 2);
				custom_indices.emplace_back(i);
				custom_indices.emplace_back(i + 2);
				custom_indices.emplace_back(i + 3);
			}
		}
	}

	/// setting the view transform yourself
	void draw(context& ctx) override
	{
		static int prev_grid_width = 0;
		static int prev_grid_height = 0;
		if (grid_width != prev_grid_width || grid_height != prev_grid_height) {
			generate_mesh();
			prev_grid_width = grid_width;
			prev_grid_height = grid_height;
		}

		ctx.push_modelview_matrix();
		cgv::math::fmat<double, 4, 4> offset = cgv::math::identity4<double>();
		offset *= cgv::math::translate4<double>(translation);
		ctx.mul_modelview_matrix(offset);

		auto& tr = cgv::render::ref_terrain_renderer(ctx, 0);
		tr.set_render_style(terrain_style);
		tr.set_position_array(ctx, custom_positions);
		tr.set_indices(ctx, custom_indices);
		tr.render(ctx, 0, custom_indices.size());

		ctx.pop_modelview_matrix();
	}
	/// overload the create gui method
	void create_gui() override
	{
		add_decorator("Terrain Demo GUI", "heading", "level=1"); // level=1 is default and can be skipped

		add_member_control(this, "grid width", grid_width, "value_slider", "min=1;max=100;log=true;ticks=true");
		add_member_control(this, "grid height", grid_height, "value_slider", "min=1;max=100;log=true;ticks=true");
		add_gui("translation", translation, "",
			"long_label=true;main_label='first';options='min=-100;max=100;ticks=true'");

		if (begin_tree_node("Terrain Settings", terrain_style, false, "level=2")) {
			align("\a");
			add_gui("terrain_render_style", terrain_style);
			align("\b");
			end_tree_node(terrain_style);
		}

	}
};

factory_registration<terrain_demo> terrain_demo_fac("terrain", "shortcut='Ctrl-Shift-T';menu_text='New/Demo/Terrain'", true);
