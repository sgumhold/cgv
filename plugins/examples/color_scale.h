#pragma once

#include <cgv/base/group.h>
#include <cgv/data/buffer2d.h>
#include <cgv/gui/event_handler.h>
#include <cgv/gui/provider.h>
#include <cgv/media/color_scale.h>
#include <cgv/media/transfer_function.h>
#include <cgv/render/color_scale_adapter.h>
#include <cgv/render/drawable.h>
#include <cgv/render/shader_library.h>
#include <cgv_overlay/color_selector.h>
#include <cgv_overlay/color_scale_legend.h>
#include <cgv_overlay/transfer_function_editor.h>

class color_scale_example :
	public cgv::base::group,			// derive from group to support child nodes (needed for overlays)
	public cgv::gui::event_handler,		// derive from event handler to receive input events
	public cgv::gui::provider,			// derive from gui provider to have gui controls
	public cgv::render::drawable		// derive from drawable to allow drawing in the GL context
{
public:
	color_scale_example();

	std::string get_type_name() const { return "color_scale_example"; }
	bool self_reflect(cgv::reflect::reflection_handler& rh) override { return false; }
	void stream_stats(std::ostream& os) override {}
	void stream_help(std::ostream& os) override {}

	bool init(cgv::render::context& ctx) override;
	void clear(cgv::render::context& ctx) override;

	void init_frame(cgv::render::context& ctx) override;
	void draw(cgv::render::context& ctx) override;

	bool handle(cgv::gui::event& event) override;
	void on_set(void* member) override;

	void create_gui() override;

private:
	/// Enum to state data source types.
	enum class SourceType {
		kHorizontalRamp,
		kPerlinNoise
	};

	/// Enum to state a data value range.
	enum class ValueRange {
		kZeroToOne,
		kMinusOneToOne,
		kOneTo100
	};

	/// Struct that holds properties of the test data.
	struct DataProperties {
		SourceType source_type = SourceType::kPerlinNoise;
		ValueRange value_range = ValueRange::kZeroToOne;
		int grid_resolution = 64;
		float noise_scale = 6.0f;
	};

	/// Enum that states color scale types.
	enum class ColorScaleType {
		kContinuous,
		kDiscrete,
		kTransferFunction
	};

	/// The minimum allowed resolution of the test data grid.
	static int min_resolution;
	/// The maximum allowed resolution of the test data grid.
	static int max_resolution;

	/// Stores the properties of the generated test data.
	DataProperties data_properties;

	/// Stores the generated test data values.
	cgv::data::buffer2d<float> values;
	/// Stores the colors mapped from the test data.
	cgv::data::buffer2d<cgv::rgb> image;

	/// Flag whether the test data in 'values' is out of date w.r.t. to the 'data_properties'.
	bool data_out_of_date = true;
	/// Flag whether the mapped colors in 'image' are out of date w.r.t. to the color scale.
	bool mapping_out_of_date = false;

	/// The texture storing the generated test data used for GPU-side mapping.
	cgv::render::texture value_texture = cgv::render::texture("flt32[R]", cgv::render::TextureFilter::TF_NEAREST, cgv::render::TextureFilter::TF_NEAREST);
	/// The texture storing the mapped colors used for CPU-side mapping.
	cgv::render::texture image_texture = cgv::render::texture("uint8[R,G,B]", cgv::render::TextureFilter::TF_NEAREST, cgv::render::TextureFilter::TF_NEAREST);
	/// Shader program to blit textures on the screen.
	cgv::render::shader_program blit_program;
	/// Custom shader program for this example to map scalars to colors and blit the result to the screen.
	cgv::render::shader_program mapping_program;

	/// If true, GPU-side mapping is used instead of CPU-side mapping.
	bool use_gpu_mapping = false;

	/// The id (a.k.a. index) to specify the used continuous color scheme.
	int continuous_color_scheme_id = 0;
	/// The id (a.k.a. index) to specify the used discrete color scheme.
	int discrete_color_scheme_id = 0;
	/// The actual used size of the chosen discrete color scheme.
	size_t discrete_color_scheme_size = 10;
	/// The type of color scale to use.
	ColorScaleType color_scale_type = ColorScaleType::kContinuous;
	/// The transfer function instance.
	std::shared_ptr<cgv::media::transfer_function> transfer_function = std::make_shared<cgv::media::transfer_function>();
	/// The continuous color scale instance.
	std::shared_ptr<cgv::media::continuous_color_scale> continuous_color_scale = std::make_shared<cgv::media::continuous_color_scale>();
	/// The discrete color scale instance.
	std::shared_ptr<cgv::media::discrete_color_scale> discrete_color_scale = std::make_shared<cgv::media::discrete_color_scale>();
	
	/// The adapter to enable using color_scales in shaders.
	cgv::render::color_scale_adapter color_scale_adapter;
	
	/// Color scale mapping parameters.
	cgv::vec2 domain = { 0.0f, 1.0f };
	bool clamp = false;
	bool reverse = false;
	bool diverging = false;
	float midpoint = 0.5f;
	float pow_exponent = 1.0f;
	float log_base = 10.0f;
	cgv::media::ContinuousMappingTransform transform = cgv::media::ContinuousMappingTransform::Linear;
	cgv::rgba unknown_color = { 1.0f, 0.0f, 1.0f, 0.0f };
	cgv::media::transfer_function::InterpolationMode color_interpolation = cgv::media::transfer_function::InterpolationMode::Linear;
	cgv::media::transfer_function::InterpolationMode opacity_interpolation = cgv::media::transfer_function::InterpolationMode::Linear;

	/// Overlay instances.
	cgv::overlay::transfer_function_editor_ptr editor;
	cgv::overlay::color_selector_ptr color_selector;
	cgv::overlay::color_scale_legend_ptr color_legend;

	void load_color_scheme_presets();
	const cgv::media::color_scale* get_active_color_scale() const;
	void create_color_scale();
	void set_color_scale_mapping_properties();

	template<typename T>
	void ensure_buffer_size(cgv::data::buffer2d<T>& buffer, size_t width, size_t height) {
		if(width != buffer.width() || height != buffer.height()) {
			buffer.clear();
			buffer.resize(width, height, T{ 0 });
		}
	}

	void on_color_map_change();

	void create_or_resize_texture(cgv::render::context& ctx, cgv::render::texture& texture) const;
	void update_value_texture(cgv::render::context& ctx);
	void update_image_texture(cgv::render::context& ctx);

	void map_values_to_color(const cgv::data::buffer2d<float>& values, cgv::data::buffer2d<cgv::rgb>& out_image);
};
