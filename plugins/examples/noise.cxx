#include <cgv/gui/provider.h>
#include <cgv/gui/dialog.h>
#include <cgv/gui/file_dialog.h>
#include <cgv/data/data_format.h>
#include <cgv/os/clipboard.h>
#include <cgv/data/data_view.h>
#include <cgv/media/image/image_writer.h>
#include <cgv/render/drawable.h>
#include <cgv/render/context.h>
#include <cgv/render/texture.h>
#include <cgv/render/attribute_array_binding.h>
#include <cgv/render/frame_buffer.h>
#include <cgv/render/shader_program.h>
#include <cgv_gl/gl/gl.h>
#include <cgv/math/ftransform.h>

#include <random>

using namespace cgv::base;
using namespace cgv::gui;
using namespace cgv::data;
using namespace cgv::render;
using namespace cgv::utils;
using namespace cgv::type;

class noise : public node, public drawable, public provider
{
	bool recreate_texture;
protected:
	bool interpolate;
	float density;
	int w, h;
	int mode;
	texture T;

public:
	noise()
	{
		set_name("noise");
		w = 256;
		h = 128;
		mode = 0;
		recreate_texture = true;
		density = 0.5f;
		interpolate = false;
		T.set_mag_filter(TF_NEAREST);
		T.set_wrap_s(cgv::render::TW_REPEAT);
		T.set_wrap_t(cgv::render::TW_REPEAT);
	}
	void on_set(void* member_ptr)
	{
		if (member_ptr == &interpolate) {
			T.set_mag_filter(interpolate ? TF_LINEAR : TF_NEAREST);
		}
		if (member_ptr == &w || member_ptr == &h || member_ptr == &mode || member_ptr == &density) {
			recreate_texture = true;
		}
		update_member(member_ptr);
		post_redraw();
	}
	void save()
	{
		std::string file_name = file_save_dialog("save noise to file", 
			"Image Files (bmp,png,jpg,tif):*.bmp;*.png;*.jpg;*.tif|All Files:*.*");
		if (file_name.empty())
			return;
		std::vector<cgv::rgba8> data(w*h);
		create_texture_data(data);
		cgv::data::data_format df(w, h, cgv::type::info::TI_UINT8, cgv::data::CF_RGBA);
		cgv::data::data_view dv(&df, &data.front());
		cgv::media::image::image_writer ir(file_name);
		ir.write_image(dv);
	}
	void copy()
	{
		std::vector<cgv::rgba8> data(w*h);
		create_texture_data(data);
		std::vector<cgv::rgb8> rgb_data(w*h);
		for (size_t i = 0; i < data.size(); ++i)
			rgb_data[i] = data[i];
		cgv::os::copy_rgb_image_to_clipboard(w, h, &rgb_data.front()[0]);
		cgv::gui::message("copied image to clipboard");
	}
	void create_gui()
	{
		add_member_control(this, "w", (cgv::type::DummyEnum&)w, "dropdown", "enums='4=4,8=8,16=16,32=32,64=64,128=128,256=256,512=512'");
		add_member_control(this, "h", (cgv::type::DummyEnum&)h, "dropdown", "enums='4=4,8=8,16=16,32=32,64=64,128=128,256=256,512=512'");
		add_member_control(this, "mode", (DummyEnum&)mode, "dropdown", "enums='grey=0;white,brown'");
		add_member_control(this, "density", density, "value_slider", "min=0;max=1;ticks=true");
		add_member_control(this, "interpolate", interpolate, "toggle");
		connect_copy(add_button("save to file")->click, cgv::signal::rebind(this, &noise::save));
		connect_copy(add_button("copy to clipboard")->click, cgv::signal::rebind(this, &noise::copy));

	}
	bool init(context& ctx)
	{
		return true;
	}
	void draw_quad(context& ctx)
	{
		const cgv::vec2 P[4] = { cgv::vec2(-1,1), cgv::vec2(-1,-1), cgv::vec2(1,1), cgv::vec2(1,-1) };
		const cgv::vec2 T[4] = { cgv::vec2(0,1), cgv::vec2(0.0f,0.0f), cgv::vec2(1,1), cgv::vec2(1.0f,0.0f) };
		attribute_array_binding::set_global_attribute_array(ctx, 0, P, 4);
		attribute_array_binding::enable_global_array(ctx, 0);
		attribute_array_binding::set_global_attribute_array(ctx, 3, T, 4);
		attribute_array_binding::enable_global_array(ctx, 3);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		attribute_array_binding::disable_global_array(ctx, 3);
		attribute_array_binding::disable_global_array(ctx, 0);
	}
	void create_texture_data(std::vector<cgv::rgba8>& data)
	{
		std::default_random_engine g;
		std::uniform_int_distribution<cgv::type::uint32_type> d(0,255);
		for (auto& c : data) {
			cgv::type::uint8_type v = d(g);
			if (mode == 1)
				v = v < density*255 ? 0 : 255;
			else if (mode == 2)
				v = (v + d(g) + d(g) + d(g))/4;
			c[0] = c[1] = c[2] = v;
			c[3] = 255;
		}
	}
	void init_frame(context& ctx)
	{
		if (!recreate_texture)
			return;
		
		std::vector<cgv::rgba8> data(w*h);
		create_texture_data(data);
		cgv::data::data_format df(w, h, cgv::type::info::TI_UINT8, cgv::data::CF_RGBA);
		cgv::data::data_view dv(&df, &data.front());
		T.create(ctx, dv);
		recreate_texture = false;
	}
	void draw(context& ctx)
	{
		glDisable(GL_CULL_FACE);
		ctx.push_modelview_matrix();
		ctx.mul_modelview_matrix(cgv::math::scale4<double>(double(w) / h, -1.0, 1.0));
		ctx.ref_default_shader_program(true).enable(ctx);
		ctx.set_color(cgv::rgb(1, 1, 1));
			T.enable(ctx);
				draw_quad(ctx);
			T.disable(ctx);
		ctx.ref_default_shader_program(true).disable(ctx);
		ctx.pop_modelview_matrix();
		glEnable(GL_CULL_FACE);
	}
};

#include <cgv/base/register.h>

/// register a factory to create new cubes
cgv::base::factory_registration<noise> noise_fac("New/Algorithms/Noise", 'N');
