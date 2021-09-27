#include "terrain_renderer.h"

#include <cgv_gl/gl/gl_tools.h>
#include <cgv/gui/provider.h>

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
	texture.min_filter = cgv::render::TF_LINEAR;
	texture.mag_filter = cgv::render::TF_LINEAR;
	texture.wrap_r = cgv::render::TW_REPEAT;
	texture.wrap_s = cgv::render::TW_REPEAT;
	texture.wrap_t = cgv::render::TW_REPEAT;
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

	//	update_texture_settings(grass_texture);
	//	if (!grass_texture.create_from_image(ctx, "../../assets/textures/Ground037_1K_Color.png")) {
	//		std::cerr << "failed to load grass texture: " << grass_texture.last_error << std::endl;
	//		return false;
	//	}
	//
	//	update_texture_settings(dirt_texture);
	//	if (!dirt_texture.create_from_image(ctx, "../../assets/textures/Ground039_1K_Color.png")) {
	//		std::cerr << "failed to load dirt texture: " << dirt_texture.last_error << std::endl;
	//		return false;
	//	}
	//
	//	update_texture_settings(rock_texture);
	//	if (!rock_texture.create_from_image(ctx, "../../assets/textures/Ground022_1K_Color.png")) {
	//		std::cerr << "failed to load rock texture: " << rock_texture.last_error << std::endl;
	//		return false;
	//	}

	return true;
}

bool terrain_renderer::enable(cgv::render::context& ctx)
{
	if (!surface_renderer::enable(ctx)) {
		return false;
	}

	if (!ref_prog().is_linked()) {
		return false;
	}

	auto& style = get_style<terrain_render_style>();
	if (style.wireframe) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}

	//	if (style.grass_texture.is_created() && style.grass_texture.enable(ctx, 0)) {
	//		return false;
	//	}
	//	if (!ref_prog().set_uniform(ctx, "grassTexture", 0)) {
	//		return false;
	//	}

	//	if (!dirt_texture.enable(ctx, 1)) {
	//		return false;
	//	}
	//	if (!ref_prog().set_uniform(ctx, "dirtTexture", 1)) {
	//		return false;
	//	}
	//
	//	if (!rock_texture.enable(ctx, 2)) {
	//		return false;
	//	}
	//	if (!ref_prog().set_uniform(ctx, "rockTexture", 2)) {
	//		return false;
	//	}

	if (!ref_prog().set_uniform(ctx, "tessellation", style.tessellation)) {
		return false;
	}
	if (!ref_prog().set_uniform(ctx, "uvScaleFactor", style.uv_scale_factor)) {
		return false;
	}

	if (!ref_prog().set_uniform(ctx, "grassLevel", style.levels.grassLevel)) {
		return false;
	}
	if (!ref_prog().set_uniform(ctx, "rockLevel", style.levels.rockLevel)) {
		return false;
	}
	if (!ref_prog().set_uniform(ctx, "blur", style.levels.blur)) {
		return false;
	}

	for (int i = 0; i < static_cast<int64_t>(style.noise_layers.size()); i++) {
		ref_prog().set_uniform(ctx, "noiseLayers[" + std::to_string(i) + "].amplitude",
							   style.noise_layers[i].amplitude);
		ref_prog().set_uniform(ctx, "noiseLayers[" + std::to_string(i) + "].frequency",
							   style.noise_layers[i].frequency);
		ref_prog().set_uniform(ctx, "noiseLayers[" + std::to_string(i) + "].enabled", style.noise_layers[i].enabled);
	}
	ref_prog().set_uniform(ctx, "numNoiseLayers", static_cast<int>(style.noise_layers.size()));
	ref_prog().set_uniform(ctx, "power", style.power);
	ref_prog().set_uniform(ctx, "bowlStrength", style.bowl_strength);
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
		p->add_member_control(b, "Show Wireframe", style->wireframe);
		p->add_member_control(b, "Tessellation", style->tessellation);
		p->add_member_control(b, "UV Scale Factor", style->uv_scale_factor);
		p->add_member_control(b, "Grass Level", style->levels.grassLevel);
		p->add_member_control(b, "Rock Level", style->levels.rockLevel);
		p->add_member_control(b, "Level Blur", style->levels.blur);

		p->add_decorator("", "separator");
		for (int i = 0; i < style->noise_layers.size(); i++) {
			auto& layer = style->noise_layers[i];
			const std::string& iStr = std::to_string(i);
			p->add_member_control(b, "Enabled " + iStr, layer.enabled);
			p->add_member_control(b, "Frequency " + iStr, layer.frequency, "value_slider", "min=0.001;max=1000;log=true;ticks=true");
			p->add_member_control(b, "Amplitude " + iStr, layer.amplitude, "value_slider", "min=0.001;max=1000;log=true;ticks=true");
		}

		return true;
	}
};

#include "gl/lib_begin.h"

CGV_API cgv::gui::gui_creator_registration<terrain_render_style_gui_creator>
	  terrain_rs_gc_reg("terrain_render_style_gui_creator");

} // namespace render
} // namespace cgv
