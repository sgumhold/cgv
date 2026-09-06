#include "color_scale.h"

#include <cgv/data/informed_ptr.h>
#include <cgv/gui/key_event.h>
#include <cgv/gui/property_string.h>
#include <cgv/media/color_scheme.h>
#include <cgv/media/named_colors.h>
#include <cgv/media/named_color_schemes.h>
#include <cgv/utils/advanced_scan.h>
#include <cgv/utils/algorithm.h>
#include <cgv/utils/dir.h>
#include <cgv/utils/file.h>
#include <cgv/utils/stopwatch.h>
#include <cgv_proc/perlin_noise.h>

using namespace cgv::render;

int color_scale_example::min_resolution = 16;
int color_scale_example::max_resolution = 2048;

color_scale_example::color_scale_example() : cgv::base::group("Color scale") {
	// Register a color scale legend overlay to show the current color mapping and domain.
	color_legend = create_and_append_child<cgv::overlay::color_scale_legend>("Legend");
	color_legend->set_stretch_mode(cgv::overlay::StretchMode::kHorizontal);
	color_legend->set_title("Data value");

	// Register a transfer function editor overlay that enables editing color scales of type cgv::media::transfer_function.
	editor = create_and_append_child<cgv::overlay::transfer_function_editor>("Transfer function");
	editor->hide();
	editor->set_stretch_mode(cgv::overlay::StretchMode::kHorizontal);
	// Disable opacity editing, since it is not needed for this example.
	editor->set_opacity_support(false);
	// Set a callback to notify this class when the transfer function was changed through the editor.
	editor->set_on_change_callback(std::bind(&color_scale_example::create_color_scale, this));
	// Set the to-be-edited transfer funciton. It is sufficient to do this once if the pointer does not change.
	editor->set_transfer_function(transfer_function);

	// Register a color selector overlay and connect it to the transfer function editor to allow chnaging control point colors with live preview.
	color_selector = create_and_append_child<cgv::overlay::color_selector>("Color selector");
	color_selector->set_alignment(cgv::overlay::Alignment::kStart, cgv::overlay::Alignment::kEnd);
	color_selector->hide();
	cgv::overlay::connect_color_selector_to_transfer_function_editor(editor, color_selector);

	load_color_scheme_presets();
}

bool color_scale_example::init(context& ctx) {
	bool success = true;

	// Load a shader for blitting the final texture on screen.
	success &= shader_library::load(ctx, blit_program, "screen_texture.glpr");
	// Load a shader for mapping a scalar texture through a device color scale and blitting the result on screen.
	// To support using color scales via the color_scale_adapter, the shader must be linked against color_scale.glsl.
	success &= shader_library::load(ctx, mapping_program, "color_mapped_texture.glpr");
	// The color scale adapter needs to be initialized before its first usage.
	success &= color_scale_adapter.init(ctx);

	// Setup the transfer function with an initial set of color control points.
	transfer_function->set_color_points({
		{ 0.0f, { 0.0f, 0.0f, 0.0f } },
		{ 0.1f, { 0.5f, 0.0f, 0.0f } },
		{ 0.75f, { 1.0f, 1.0f, 0.0f } },
		{ 1.0f, { 0.5f, 1.0f, 1.0f } },
	});
	
	// Make sure the selected color scale is created and the settings are synchronized with the GUI.
	create_color_scale();
	set_color_scale_mapping_properties();
	
 	ctx.set_bg_color(0.2f, 0.2f, 0.2f, 1.0f);
	return success;
}

void color_scale_example::clear(context& ctx) {
	image.clear();
	blit_program.destruct(ctx);
	mapping_program.destruct(ctx);
	color_scale_adapter.destruct(ctx);
}

void color_scale_example::init_frame(context& ctx) {
	// Make sure the used textures are created with the correct size.
	create_or_resize_texture(ctx, value_texture);
	create_or_resize_texture(ctx, image_texture);

	// Recreate the test data if it is out of date with the generation settings.
	if(data_out_of_date) {
		// Mkae sure that the used buffers are of correct size.
		ensure_buffer_size(image, data_properties.grid_resolution, data_properties.grid_resolution);
		ensure_buffer_size(values, data_properties.grid_resolution, data_properties.grid_resolution);

		// Generate test data values in the range [-1,1] for every cell in the grid.
		for(size_t y = 0; y < values.height(); ++y) {
			for(size_t x = 0; x < values.width(); ++x) {
				cgv::vec2 uv = { static_cast<float>(x), static_cast<float>(y) };
				uv /= static_cast<float>(data_properties.grid_resolution - 1);

				switch(data_properties.source_type) {
				case SourceType::kHorizontalRamp:
					values(x, y) = 2.0f * uv.x() - 1.0f;
					break;
				case SourceType::kPerlinNoise:
					uv *= data_properties.noise_scale;
					// Perlin noise values already range from -1 to 1.
					values(x, y) = cgv::proc::perlin_noise_3d(uv.x(), uv.y(), 0.0f);
					break;
				default:
					values(x, y) = 0.0f;
					break;
				}
			}
		}

		// Remap the test values if requested.
		switch(data_properties.value_range) {
		case ValueRange::kZeroToOne:
			std::for_each(values.begin(), values.end(), [](float& value) { value = 0.5f * value + 0.5f; });
			break;
		case ValueRange::kOneTo100:
			std::for_each(values.begin(), values.end(), [](float& value) { value = cgv::math::map(value, -1.0f, 1.0f, 1.0f, 100.0f); });
			break;
		default:
			break;
		}
		
		// Print information about the generated value range which might not fully cover the requested range in the case of perlin noise.
		auto pair = std::minmax_element(values.begin(), values.end());
		float min = *pair.first;
		float max = *pair.second;
		std::cout << "Data range = [" << min << ", " << max << "]" << std::endl;
		
		update_value_texture(ctx);
		data_out_of_date = false;
		mapping_out_of_date = true;
	}

	// Map the values to colors if requested.
	if(mapping_out_of_date && !use_gpu_mapping) {
		cgv::utils::stopwatch s(true);
		map_values_to_color(values, image);
		double seconds = s.get_elapsed_time();
		std::cout << "Mapping done in " << seconds << " s" << std::endl;
		update_image_texture(ctx);
		mapping_out_of_date = false;
	}
}

void color_scale_example::draw(context& ctx) {
	if(!image_texture.is_created())
		return;
	
	// Setup the rendering. Disable depth testing and set gamma to 1 to show the colors unaltered.
	ctx.push_depth_test_state();
	ctx.disable_depth_test();

	float gamma = ctx.get_gamma();
	ctx.set_gamma(1.0f);

	ctx.begin_attribute_less_rendering();

	if(use_gpu_mapping) {
		// For GPU-side mapping, bind the value texture and enable the color scale adapter to allow for live mapping in the shader.
		value_texture.enable(ctx, 0);

		mapping_program.enable(ctx);
		mapping_program.set_uniform(ctx, "value_texture", 0);
		color_scale_adapter.set_uniforms_in_program(ctx, mapping_program, 1);
		color_scale_adapter.enable(ctx, 1);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		color_scale_adapter.disable(ctx);
		mapping_program.disable(ctx);

		value_texture.disable(ctx);
	} else {
		// For CPU-side mapping, simply render the created texture containing the mapped colors.
		image_texture.enable(ctx, 0);

		blit_program.enable(ctx);
		blit_program.set_uniform(ctx, "color_tex", 0);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		blit_program.disable(ctx);

		image_texture.disable(ctx);
	}
	
	ctx.end_attribute_less_rendering();

	ctx.set_gamma(gamma);
	ctx.pop_depth_test_state();
}

bool color_scale_example::handle(cgv::gui::event& event) {
	if(event.get_kind() == cgv::gui::EventId::EID_MOUSE) {
		// Print out the scalar value of the grid cell pointed to by the mouse pointer when holding CTRL.
		auto& mouse_event = dynamic_cast<cgv::gui::mouse_event&>(event);
		if(mouse_event.get_action() == cgv::gui::MouseAction::MA_MOVE && mouse_event.get_modifiers() == cgv::gui::EventModifier::EM_CTRL) {
			if(auto ctx = get_context()) {
				cgv::ivec4 viewport = ctx->get_window_transformation_array().back().viewport;
				cgv::vec2 viewport_size = { static_cast<float>(viewport.z()), static_cast<float>(viewport.w()) };
				cgv::vec2 mouse_position = { static_cast<float>(mouse_event.get_x()), static_cast<float>(mouse_event.get_y()) };
				cgv::vec2 normalized_position = mouse_position / viewport_size;
				normalized_position.y() = 1.0f - normalized_position.y();

				size_t x = static_cast<size_t>(normalized_position.x() *= static_cast<float>(values.width()));
				size_t y = static_cast<size_t>(normalized_position.y() *= static_cast<float>(values.height()));

				x = std::min(x, values.width());
				y = std::min(y, values.height());

				if(x < values.width() && y < values.height()) {
					std::cout << "Value at " << x << ", " << y << " = " << values(x, y) << std::endl;
					return true;
				}
			}
		}
	}

	return false;
}

void color_scale_example::on_set(void* member) {
	const cgv::data::informed_ptr ptr(member);
	
	if(ptr.points_to_member_of(data_properties)) {
		data_properties.grid_resolution = cgv::math::clamp(data_properties.grid_resolution, min_resolution, max_resolution);
		update_member(&data_properties.grid_resolution);
		data_out_of_date = true;
	}

	if(ptr.points_to(color_scale_type)) {
		if(editor)
			editor->set_visibility(color_scale_type == ColorScaleType::kTransferFunction);
		create_color_scale();
		post_recreate_gui();
	}

	if(ptr.points_to_one_of(continuous_color_scheme_id, discrete_color_scheme_id, discrete_color_scheme_size)) {
		discrete_color_scheme_size = cgv::math::clamp(discrete_color_scheme_size, size_t(3), size_t(20));
		create_color_scale();
	}

	if(ptr.points_to_one_of(domain[0], domain[1], clamp, reverse, diverging, midpoint, pow_exponent, log_base, transform, unknown_color, color_interpolation, opacity_interpolation)) {
		set_color_scale_mapping_properties();
	}
	
	if(ptr.points_to(use_gpu_mapping))
		mapping_out_of_date = true;

	update_member(member);
	post_redraw();
}

void color_scale_example::create_gui() {
	cgv::gui::property_string options;
	options.add("min", min_resolution);
	options.add("max", max_resolution);
	options.add("step", 2);
	add_member_control(this, "Source", data_properties.source_type, "dropdown", "enums='Horizontal ramp,Perlin noise'");
	add_member_control(this, "Value Range", data_properties.value_range, "dropdown", "enums='0 to 1,-1 to 1,1 to 100'");
	add_member_control(this, "Resolution", data_properties.grid_resolution, "value_slider", options);
	add_member_control(this, "Noise scale", data_properties.noise_scale, "value_slider", "min=1;max=32;step=0.01");

	add_member_control(this, "GPU mapping", use_gpu_mapping, "check");
	
	add_decorator("Color scale", "heading", "level=4");
	add_member_control(this, "Type", color_scale_type, "dropdown", "enums='Continuous,Discrete,Transfer function'");
	if(color_scale_type == ColorScaleType::kContinuous) {
		std::string dropdown_options = cgv::utils::join(cgv::media::get_global_continuous_color_scheme_registry().get_names(), ",");
		add_member_control(this, "Preset", reinterpret_cast<cgv::type::DummyEnum&>(continuous_color_scheme_id), "dropdown", "enums='" + dropdown_options + "'");
	} else if(color_scale_type == ColorScaleType::kDiscrete) {
		std::string dropdown_options = cgv::utils::join(cgv::media::get_global_discrete_color_scheme_registry().get_names(), ",");
		add_member_control(this, "Preset", reinterpret_cast<cgv::type::DummyEnum&>(discrete_color_scheme_id), "dropdown", "enums='" + dropdown_options + "'");
		add_member_control(this, "Size", discrete_color_scheme_size, "value", "min=3;max=20;step=1");
	}
	
	add_member_control(this, "Domain min", domain[0], "value_slider", "min=-2;max=2;step=0.01");
	add_member_control(this, "Domain max", domain[1], "value_slider", "min=-2;max=2;step=0.01");
	add_member_control(this, "Clamp", clamp, "check");
	add_member_control(this, "Transform", transform, "dropdown", "enums='Linear,Pow,Log'");
	add_member_control(this, "Diverging", diverging, "check");
	add_member_control(this, "Midpoint", midpoint, "value_slider", "min=-2;max=2;step=0.01");
	add_member_control(this, "Pow exponent", pow_exponent, "value_slider", "min=0;max=5;step=0.01");
	add_member_control(this, "Log base", log_base, "value_slider", "min=2;max=10;step=0.01");
	add_member_control(this, "Reverse", reverse, "check");
	add_member_control(this, "Unknown color", unknown_color);

	if(color_scale_type == ColorScaleType::kTransferFunction) {
		add_member_control(this, "Color interpolation", color_interpolation, "dropdown", "enums='Step,Linear,Smooth'");
		add_member_control(this, "Opacity interpolation", opacity_interpolation, "dropdown", "enums='Step,Linear,Smooth'");

		if(editor)
			inline_object_gui(editor);
	}

	if(color_legend)
		inline_object_gui(color_legend);
}

void color_scale_example::load_color_scheme_presets() {
	// This example uses the global color scheme registries to manage known color schemes.
	cgv::media::continuous_color_scheme_registry& continuous_schemes = cgv::media::get_global_continuous_color_scheme_registry();
	cgv::media::discrete_color_scheme_registry& discrete_schemes = cgv::media::get_global_discrete_color_scheme_registry();

	// Load the built-in color scheme presets.
	size_t continuous_count = cgv::media::load_continuous_color_scheme_presets(continuous_schemes);
	size_t discrete_count = cgv::media::load_discrete_color_scheme_presets(discrete_schemes);
	
	std::cout << "Loaded " << continuous_count << "/" << discrete_count << " continuous/discrete color scheme presets." << std::endl;
	namespace colors = cgv::media::colors;
	// Register a custom continuous scheme using a uniform-spaced linear interpolation of three colors.
	continuous_schemes.add("rdgnbu", cgv::media::continuous_color_scheme::linear({ colors::red, colors::green, colors::blue }));
	// Register a custom discrete scheme of size three.
	discrete_schemes.add("traffic_light", cgv::media::discrete_color_scheme({ colors::green, colors::yellow, colors::red }));
}

const cgv::media::color_scale* color_scale_example::get_active_color_scale() const {
	// Return the currently selected color scale.
	switch(color_scale_type) {
	case ColorScaleType::kContinuous:
		return continuous_color_scale.get();
	case ColorScaleType::kDiscrete:
		return discrete_color_scale.get();
	case ColorScaleType::kTransferFunction:
		return transfer_function.get();
	}
	return nullptr;
}

void color_scale_example::create_color_scale() {
	// Create a color scale of the type selected in the GUI.
	// Continuous and discrete color scales are created from the chosen scheme retrieved from the respective global registry.
	// The transfer function is simply managed as a class member.
	switch(color_scale_type) {
	case ColorScaleType::kContinuous:
		continuous_color_scale->set_scheme(cgv::media::get_global_continuous_color_scheme_registry().get(continuous_color_scheme_id));
		// Make sure the legend and adapter use the selected color scale. This only needs to be done when the actual color scale
		// instance as given by the pointer changes. Changes to a persistent color scale instance are automatically checked by the legend and adapter.
		// The color scale adapter needs a device_color_scale instance which can simply be constructed from the respective host-side variant.
		color_legend->set_color_scale(continuous_color_scale);
		color_scale_adapter.set_color_scale(std::make_shared<device_continuous_color_scale>(continuous_color_scale));
		break;
	case ColorScaleType::kDiscrete:
		discrete_color_scale->set_scheme(cgv::media::get_global_discrete_color_scheme_registry().get(discrete_color_scheme_id), discrete_color_scheme_size);
		color_legend->set_color_scale(discrete_color_scale);
		color_scale_adapter.set_color_scale(std::make_shared<device_discrete_color_scale>(discrete_color_scale));
		break;
	case ColorScaleType::kTransferFunction:
		color_legend->set_color_scale(transfer_function);
		color_scale_adapter.set_color_scale(std::make_shared<device_transfer_function>(transfer_function));
		break;
	}

	// Raise the flag to update the color mapping, since the scale has changed.
	mapping_out_of_date = true;
	post_redraw();
}

void color_scale_example::set_color_scale_mapping_properties() {
	// This method simply synchronizes the color scale settings with the GUI controls.
	continuous_color_scale->set_domain(domain);
	continuous_color_scale->set_clamped(clamp);
	continuous_color_scale->set_reversed(reverse);
	continuous_color_scale->set_diverging(diverging);
	continuous_color_scale->set_midpoint(midpoint);
	continuous_color_scale->set_pow_exponent(pow_exponent);
	continuous_color_scale->set_log_base(log_base);
	continuous_color_scale->set_transform(transform);
	continuous_color_scale->set_unknown_color(unknown_color);
	
	discrete_color_scale->set_domain(domain);
	discrete_color_scale->set_clamped(clamp);
	discrete_color_scale->set_reversed(reverse);
	discrete_color_scale->set_unknown_color(unknown_color);

	transfer_function->rescale(domain);
	transfer_function->set_clamped(clamp);
	transfer_function->set_reversed(reverse);
	transfer_function->set_unknown_color(unknown_color);
	transfer_function->set_color_interpolation(color_interpolation);
	transfer_function->set_opacity_interpolation(opacity_interpolation);

	// The editor needs to be notified whenever the transfer function is changed from the outside.
	editor->notify_transfer_function_change();

	mapping_out_of_date = true;
}

void color_scale_example::on_color_map_change() {
	mapping_out_of_date = true;
}

void color_scale_example::create_or_resize_texture(context& ctx, cgv::render::texture& texture) const {
	// Create the texture only if it is not created or the size has changed.
	bool size_out_of_date = data_properties.grid_resolution != texture.get_width() || data_properties.grid_resolution != texture.get_height();
	if(size_out_of_date || !texture.is_created())
		texture.create(ctx, TextureType::TT_2D, data_properties.grid_resolution, data_properties.grid_resolution);
}

void color_scale_example::update_value_texture(context& ctx) {
	cgv::data::data_format format(value_texture.get_width(), value_texture.get_height(), cgv::type::info::TI_FLT32, cgv::data::CF_R);
	cgv::data::data_view data(&format, values.data());
	value_texture.create(ctx, data);
}

void color_scale_example::update_image_texture(context& ctx) {
	cgv::data::data_format format(image_texture.get_width(), image_texture.get_height(), cgv::type::info::TI_FLT32, cgv::data::CF_RGB);
	cgv::data::data_view data(&format, image.data());
	image_texture.create(ctx, data);
}

void color_scale_example::map_values_to_color(const cgv::data::vector2d<float>& values, cgv::data::vector2d<cgv::rgb>& out_image) {
	// Map all scalar test data 'values' through the active color scale and store the resulting colors in the 'image' buffer.
	const cgv::media::color_scale* color_scale = get_active_color_scale();
	std::transform(values.begin(), values.end(), out_image.begin(), [color_scale](float value) {
		return color_scale->get_mapped_color(value);
	});
}

#include <cgv/base/register.h>
extern cgv::base::factory_registration<color_scale_example> color_scale_example_reg("Color scale", "shortcut='Ctrl-C';menu_text='New/Media/Color scale'", true);
