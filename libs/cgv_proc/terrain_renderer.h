#pragma once

#include <cgv_gl/surface_renderer.h>

#include "lib_begin.h"

namespace cgv {
namespace render {

/// Layer of noise, defined by its frequency and amplitude
struct NoiseLayer
{
	bool enabled = true;
	float frequency = 1.0f;
	float amplitude = 1.0f;

	NoiseLayer() = default;
	NoiseLayer(float frequency, float amplitude) : frequency(frequency), amplitude(amplitude) {}
};

/// determines at which relative height the different terrain types transition
struct TerrainLevels
{
	float grassLevel = 0.4f;
	float rockLevel = 0.6f;
	float blur = 0.05f;
};

struct CGV_API terrain_render_style : public cgv::render::surface_render_style
{
	int seed = 1337;
	bool should_apply_power = true;
	float power = 1.1f;
	bool should_apply_bowl = true;
	float bowl_strength = 20.0f;
	bool should_apply_platform = true;
	float platform_height = 0.15f;

	std::vector<NoiseLayer> noise_layers;
	TerrainLevels levels;

	bool  wireframe = false;
	float tessellation = 60.0f;
	float uv_scale_factor_grass = 20.0f;
	float uv_scale_factor_dirt  = 20.0f;
	float uv_scale_factor_rock  = 20.0f;
	float aspect_ratio_grass    = 1.0f;
	float aspect_ratio_dirt     = 1.0f;
	float aspect_ratio_rock     = 1.0f;
	float damp_factor_grass     = 1.0f;
	float damp_factor_dirt      = 1.0f;
	float damp_factor_rock      = 1.0f;

	bool use_grass_texture = true;
	bool use_dirt_texture = true;
	bool use_rock_texture = true;

	int get_color_implementation = 2;

	mutable cgv::render::texture grass_texture = {};
	mutable cgv::render::texture dirt_texture = {};
	mutable cgv::render::texture rock_texture = {};

	bool load_default_textures(cgv::render::context& ctx);
};

/// renderer that supports point splatting
class CGV_API terrain_renderer : public cgv::render::surface_renderer
{
  protected:
	/// return the default shader program name
	std::string get_default_prog_name() const override { return "terrain.glpr"; }
	/// create and return the default render style
	render_style* create_render_style() const override { return new terrain_render_style(); }

  public:
	bool validate_attributes(const cgv::render::context& ctx) const override;
	bool enable(cgv::render::context& ctx) override;
	bool disable(cgv::render::context& ctx) override;
	void draw(cgv::render::context& ctx, size_t start, size_t count, bool use_strips, bool use_adjacency,
			  uint32_t strip_restart_index) override;
};

struct CGV_API terrain_render_style_reflect : public terrain_render_style
{
	bool self_reflect(cgv::reflect::reflection_handler& rh);
};
extern CGV_API cgv::reflect::extern_reflection_traits<terrain_render_style, terrain_render_style_reflect>
get_reflection_traits(const terrain_render_style&);

//! reference to a singleton terrain renderer that is shared among drawables
/*! the second parameter is used for reference counting. Use +1 in your init method,
	-1 in your clear method and default 0 argument otherwise. If internal reference
	counter decreases to 0, singleton renderer is destructed. */
extern CGV_API terrain_renderer& ref_terrain_renderer(cgv::render::context& ctx, int ref_count_change = 0);

} // namespace render
} // namespace cgv

#include <cgv/config/lib_end.h>
