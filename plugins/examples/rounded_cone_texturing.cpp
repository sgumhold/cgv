#include <cgv/base/node.h>
#include <cgv/gui/provider.h>
#include <cgv/math/ftransform.h>
#include <cgv/media/image/image.h>
#include <cgv/media/image/image_reader.h>
#include <cgv/render/drawable.h>
#include <cgv/render/texture.h>
#include <cgv_glutil/cone_render_data.h>

class rounded_cone_texturing : public cgv::base::node, public cgv::render::drawable, public cgv::gui::provider {
protected:
	cgv::render::rounded_cone_render_style cone_style;
	cgv::glutil::cone_render_data<> rd;

	cgv::render::texture tex;

public:
	rounded_cone_texturing() : cgv::base::node("rounded cone texturing test") {		
		cone_style.radius = 0.1f;
		cone_style.surface_color = rgb(1.0f, 0.5f, 0.2f);
		cone_style.enable_texturing = true;
		cone_style.texture_blend_mode = cgv::render::rounded_cone_render_style::TBM_TINT;
		rd = cgv::glutil::cone_render_data<>(true);
	}
	void on_set(void* member_ptr) {
		post_redraw();
		update_member(member_ptr);
	}
	std::string get_type_name() const {
		return "rounded_cone_texturing";
	}
	void clear(cgv::render::context& ctx) {
		cgv::render::ref_rounded_cone_renderer(ctx, -1);
		rd.destruct(ctx);
		tex.destruct(ctx);
	}
	bool init(cgv::render::context& ctx) {

		cgv::render::ref_rounded_cone_renderer(ctx, 1);
		if(!rd.init(ctx))
			return false;

		for(unsigned i = 0; i < 3; ++i) {
			float x = 0.5f * static_cast<float>(i + 1);
			float y = 0.3f * static_cast<float>(i);
			rd.add(vec3(-x, y, 0.0f), vec3(x, y, 0.0f));
		}
		rd.set_out_of_date();

		cgv::data::data_format tex_format;
		cgv::media::image::image_reader image(tex_format);
		cgv::data::data_view tex_data;

		std::string file_name = "res://plus.png";
		if(!image.read_image(file_name, tex_data)) {
			std::cout << "Error: Could not read image file " << file_name << std::endl;
			return false;
		} else {
			tex.create(ctx, tex_data, 0);
			tex.set_min_filter(cgv::render::TextureFilter::TF_LINEAR);
			tex.set_mag_filter(cgv::render::TextureFilter::TF_LINEAR);
			tex.set_wrap_s(cgv::render::TextureWrap::TW_REPEAT);
			tex.set_wrap_t(cgv::render::TextureWrap::TW_REPEAT);
		}
		image.close();
		return true;
	}
	void init_frame(cgv::render::context& ctx) {
		
	}
	void draw(cgv::render::context& ctx) {
		if(!tex.is_created())
			return;

		auto& rcr = ref_rounded_cone_renderer(ctx);
		rcr.set_albedo_texture(&tex);
		rd.render(ctx, rcr, cone_style);
	}
	void create_gui() {
		add_decorator("rounded cone texturing", "heading");
		if(begin_tree_node("cone style", cone_style, true)) {
			align("\a");
			add_gui("", cone_style);
			align("\b");
			end_tree_node(cone_style);
		}
	}
};

#include <cgv/base/register.h>

/// register a factory to create new rounded cone texturing tests
cgv::base::factory_registration<rounded_cone_texturing> test_rounded_cone_texturing_fac("new/demo/rounded_cone_texturing", '1');
