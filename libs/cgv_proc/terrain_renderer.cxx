#include "terrain_renderer.h"

#include <cgv_gl/gl/gl_tools.h>
#include <cgv/gui/provider.h>
#include <cgv/gui/dialog.h>
#include <cgv/gui/file_dialog.h>
#include <cgv/defines/quote.h>
#include <cgv/base/import.h>

namespace cgv {
namespace render {

terrain_renderer& ref_terrain_renderer(cgv::render::context& ctx, int ref_count_change)
{
	static int ref_count = 0;
	static terrain_renderer r;
	r.manage_singleton(ctx, "terrain_renderer", ref_count, ref_count_change);
	return r;
}

cgv::render::render_style* terrain_renderer::create_render_style() const
{
	return new terrain_render_style();
}

bool terrain_renderer::validate_attributes(const cgv::render::context& ctx) const
{
	const auto& style = get_style<terrain_render_style>();
	return surface_renderer::validate_attributes(ctx);
}

void update_texture_settings(cgv::render::texture& texture)
{
	texture.set_min_filter(cgv::render::TF_ANISOTROP, 8.0f);
	texture.set_mag_filter(cgv::render::TF_LINEAR);
	texture.set_wrap_s(cgv::render::TW_REPEAT);
	texture.set_wrap_t(cgv::render::TW_REPEAT);
}

bool find_or_download_texture(cgv::render::context& ctx, const std::string& name, cgv::render::texture& tex, const std::string& file_name, const std::string& url)
{
	std::string file_path = cgv::base::find_or_download_data_file(file_name, "cmD", url, "Dcm", "terrain_renderer", "textures", QUOTE_SYMBOL_VALUE(INPUT_DIR), 
		cgv::base::user_feedback(&cgv::gui::message, &cgv::gui::question, &cgv::gui::directory_save_dialog));
	if (file_path.empty()) {
		std::cerr << "could not find " << name << " texture" << std::endl;
		return false;
	}
	if (tex.create_from_image(ctx, file_path)) {
		tex.generate_mipmaps(ctx);
		update_texture_settings(tex);
		return true;
	}
	std::cerr << "failed to load " << name << " texture: " << tex.last_error << std::endl;
	return false;
}

bool terrain_render_style::load_default_textures(cgv::render::context& ctx)
{
	bool success = true;
	// old names:
	//    grass texture: "../../assets/textures/Ground037_1K_Color.png"
	//    dirt texture: "../../assets/textures/Ground039_1K_Color.png"
	//    rock texture: "../../assets/textures/Ground022_1K_Color.png"
	if (find_or_download_texture(ctx, "grass", grass_texture, "terrain_grass_texture.jpg", "https://3djungle.net/download/texture/XLs0Lg==/")) {
		damp_factor_grass = 0.8f;
		uv_scale_factor_grass = 10.0f;
		aspect_ratio_grass = 1.0f;
	}
	else 
		success = false;

	if (find_or_download_texture(ctx, "dirt", dirt_texture, "terrain_dirt_texture.jpg", "https://3djungle.net/download/texture/Wr42JQ==/")) {
		damp_factor_dirt = 0.6f;
		uv_scale_factor_dirt = 40.0f;
		aspect_ratio_dirt = 1.0f;
	}
	else
		success = false;
	if (find_or_download_texture(ctx, "rock", rock_texture, "terrain_rock_texture.jpg", "https://3djungle.net/download/texture/W7k0Lw==/")) {
		damp_factor_rock = 0.8f;
		uv_scale_factor_rock = 20.0f;
		aspect_ratio_rock = 2.0f;
	}
	else
		success = false;
	return success;
}

bool terrain_renderer::init(cgv::render::context& ctx)
{
	surface_renderer::init(ctx);
	if (!ref_prog().is_created()) {
		if (!ref_prog().build_program(ctx, "terrain.glpr", true)) {
			std::cerr << "ERROR in terrain_renderer::init() ... could not build program terrain.glpr" << std::endl;
			return false;
		}
	}
	return true;
}

bool terrain_renderer::enable(cgv::render::context& ctx)
{
	if (!ref_prog().is_linked()) {
		return false;
	}
	if (!surface_renderer::enable(ctx)) {
		return false;
	}
	auto& style = get_style<terrain_render_style>();
	if (style.wireframe) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	ref_prog().set_uniform(ctx, "get_color_implementation", style.get_color_implementation);
	ref_prog().set_uniform(ctx, "useGrassTexture", 
		style.use_grass_texture && style.grass_texture.is_created() && style.grass_texture.enable(ctx, 0) && ref_prog().set_uniform(ctx, "grassTexture", 0));
	ref_prog().set_uniform(ctx, "useDirtTexture",
		style.use_dirt_texture && style.dirt_texture.is_created() && style.dirt_texture.enable(ctx, 1) && ref_prog().set_uniform(ctx, "dirtTexture", 1));
	ref_prog().set_uniform(ctx, "useRockTexture",
		style.use_rock_texture && style.rock_texture.is_created() && style.rock_texture.enable(ctx, 2) && ref_prog().set_uniform(ctx, "rockTexture", 2));
	ref_prog().set_uniform(ctx, "tessellation", style.tessellation);
	ref_prog().set_uniform(ctx, "uv_scale_factor", vec3(style.uv_scale_factor_grass, style.uv_scale_factor_dirt, style.uv_scale_factor_rock));
	ref_prog().set_uniform(ctx, "aspect_ratio", vec3(style.aspect_ratio_grass, style.aspect_ratio_dirt, style.aspect_ratio_rock));
	ref_prog().set_uniform(ctx, "damp_factor", vec3(style.damp_factor_grass, style.damp_factor_dirt, style.damp_factor_rock));
	ref_prog().set_uniform(ctx, "grassLevel", style.levels.grassLevel);
	ref_prog().set_uniform(ctx, "rockLevel", style.levels.rockLevel);
	ref_prog().set_uniform(ctx, "blur", style.levels.blur);

	for (int i = 0; i < static_cast<int64_t>(style.noise_layers.size()); i++) {
		ref_prog().set_uniform(ctx, "noiseLayers[" + std::to_string(i) + "].amplitude",
							   style.noise_layers[i].amplitude);
		ref_prog().set_uniform(ctx, "noiseLayers[" + std::to_string(i) + "].frequency",
							   style.noise_layers[i].frequency);
		ref_prog().set_uniform(ctx, "noiseLayers[" + std::to_string(i) + "].enabled", style.noise_layers[i].enabled);
	}
	ref_prog().set_uniform(ctx, "numNoiseLayers", static_cast<int>(style.noise_layers.size()));
	ref_prog().set_uniform(ctx, "shouldApplyPower", style.should_apply_power);
	ref_prog().set_uniform(ctx, "power", style.power);
	ref_prog().set_uniform(ctx, "shouldApplyBowl", style.should_apply_bowl);
	ref_prog().set_uniform(ctx, "bowlStrength", style.bowl_strength);
	ref_prog().set_uniform(ctx, "shouldApplyPlatform", style.should_apply_platform);
	ref_prog().set_uniform(ctx, "platformHeight", style.platform_height);
	ref_prog().set_uniform(ctx, "seed", style.seed);

	return true;
}

bool terrain_renderer::disable(cgv::render::context& ctx)
{
	auto& style = get_style<terrain_render_style>();
	if (style.wireframe) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	if (style.use_grass_texture && style.grass_texture.is_created())
		style.grass_texture.disable(ctx);
	if (style.use_dirt_texture && style.dirt_texture.is_created())
		style.dirt_texture.disable(ctx);
	if (style.use_rock_texture && style.rock_texture.is_created())
		style.rock_texture.disable(ctx);

	return surface_renderer::disable(ctx);
}

bool terrain_render_style_reflect::self_reflect(cgv::reflect::reflection_handler& rh)
{
	return rh.reflect_base(*static_cast<terrain_render_style*>(this));
}

void terrain_renderer::draw(cgv::render::context& ctx, size_t start, size_t count, bool use_strips, bool use_adjacency,
							uint32_t strip_restart_index)
{
	glPatchParameteri(GL_PATCH_VERTICES, 3);
	draw_impl(ctx, cgv::render::PT_PATCHES, start, count, false, false, -1);
}

cgv::reflect::extern_reflection_traits<terrain_render_style, terrain_render_style_reflect>
get_reflection_traits(const terrain_render_style&)
{
	return {};
}


} // namespace render
} // namespace cgv

struct terrain_render_style_gui_creator : public cgv::gui::gui_creator
{
	/// attempt to create a gui and return whether this was successful
	bool create(cgv::gui::provider* p, const std::string& label, void* value_ptr, const std::string& value_type,
		const std::string& gui_type, const std::string& options, bool*) override
	{
		if (value_type != cgv::type::info::type_name<cgv::render::terrain_render_style>::get_name())
			return false;

		auto* style = reinterpret_cast<cgv::render::terrain_render_style*>(value_ptr);
		auto* b = dynamic_cast<cgv::base::base*>(p);
		p->add_member_control(b, "show wireframe", style->wireframe, "check");
		p->add_member_control(b, "tessellation", style->tessellation, "value_slider", "min=1;max=80;log=true;ticks=true");
		p->add_member_control(b, "should apply power", style->should_apply_power, "check");
		p->add_member_control(b, "power", style->power, "value_slider", "min=0.9;max=1.5;ticks=true");
		p->add_member_control(b, "should apply bowl", style->should_apply_bowl, "check");
		p->add_member_control(b, "bowl strength", style->bowl_strength, "value_slider", "min=0;max=500;ticks=true;log=true");
		p->add_member_control(b, "should apply platform height", style->should_apply_platform, "check");
		p->add_member_control(b, "platform height", style->platform_height, "value_slider", "min=0;max=0.5;ticks=true");
		if (p->begin_tree_node("Textures", style->levels.grassLevel, false, "level=3")) {
			p->align("\a");
			p->add_member_control(b, "grass level", style->levels.grassLevel, "value_slider", "min=0;max=2;ticks=true");
			p->add_member_control(b, "rock level", style->levels.rockLevel, "value_slider", "min=0;max=2;ticks=true");
			p->add_member_control(b, "level blur", style->levels.blur, "value_slider", "min=0;max=0.2;ticks=true");
			p->add_member_control(b, "get_color_implementation", (cgv::type::DummyEnum&)style->get_color_implementation, "dropdown", "enums='if,step,clamp,smoothstep'");
			p->add_decorator("grass", "heading", "level=3");
			p->add_member_control(b, "use texture", style->use_grass_texture, "check");
			p->add_member_control(b, "damp factor", style->damp_factor_grass, "value_slider", "min=0;max=2;ticks=true");
			p->add_member_control(b, "uv scale factor", style->uv_scale_factor_grass, "value_slider", "min=1;max=500;log=true;ticks=true");
			p->add_member_control(b, "aspect ratio", style->aspect_ratio_grass, "value_slider", "min=0.2;max=5;log=true;ticks=true");
			p->add_decorator("dirt", "heading", "level=3");
			p->add_member_control(b, "use texture", style->use_dirt_texture, "check");
			p->add_member_control(b, "damp factor", style->damp_factor_dirt, "value_slider", "min=0;max=2;ticks=true");
			p->add_member_control(b, "uv scale factor", style->uv_scale_factor_dirt, "value_slider", "min=1;max=500;log=true;ticks=true");
			p->add_member_control(b, "aspect ratio", style->aspect_ratio_dirt, "value_slider", "min=0.2;max=5;log=true;ticks=true");
			p->add_decorator("rock", "heading", "level=3");
			p->add_member_control(b, "use texture", style->use_rock_texture, "check");
			p->add_member_control(b, "damp factor", style->damp_factor_rock, "value_slider", "min=0;max=2;ticks=true");
			p->add_member_control(b, "uv scale factor", style->uv_scale_factor_rock, "value_slider", "min=1;max=500;log=true;ticks=true");
			p->add_member_control(b, "aspect ratio", style->aspect_ratio_rock, "value_slider", "min=0.2;max=5;log=true;ticks=true");
			p->align("\b");
			p->end_tree_node(style->levels.grassLevel);
		}

		if (p->begin_tree_node("Noise Layers", style->noise_layers, false, "level=3")) {
			p->align("\a");
			for (int i = 0; i < style->noise_layers.size(); i++) {
				auto& layer = style->noise_layers[i];
				const std::string& iStr = std::to_string(i);
				p->add_member_control(b, "Enabled " + iStr, layer.enabled, "check");
				p->add_member_control(b, "Frequency " + iStr, layer.frequency, "value_slider",
					"min=0.001;max=1000;log=true;ticks=true");
				p->add_member_control(b, "Amplitude " + iStr, layer.amplitude, "value_slider",
					"min=0.001;max=1000;log=true;ticks=true");
			}
			p->align("\b");
			p->end_tree_node(style->noise_layers);
		}

		if (p->begin_tree_node("Surface", style->damp_factor_dirt, false, "level=3")) {
			p->align("\a");
			p->add_gui<cgv::render::surface_render_style>("surface_render_style",
				*reinterpret_cast<cgv::render::surface_render_style*>(value_ptr));
			p->align("\b");
			p->end_tree_node(style->damp_factor_dirt);
		}
		return true;
	}
};

#include "lib_begin.h"

CGV_API cgv::gui::gui_creator_registration<terrain_render_style_gui_creator>
terrain_rs_gc_reg("terrain_render_style_gui_creator");

#ifdef REGISTER_SHADER_FILES
#include <cgv_proc_shader_inc.h>
#endif
