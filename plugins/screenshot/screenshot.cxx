#include "screenshot.h"

#include <cgv/base/find_action.h>
#include <cgv/gui/dialog.h>
#include <cgv/gui/theme_info.h>
#include <cgv/render/clipped_view.h>
#include <cgv/render/view.h>
#include <cgv/utils/algorithm.h>
#include <cgv/utils/convert_string.h>
#include <cgv/utils/dir.h>
#include <tinyxml2/tinyxml2.h>
#include <cgv_xml/query.h>

using namespace cgv::render;

const std::array<cgv::ivec2, 7> screenshot::resolution_presets = {{
		{ -1, -1 },
		{ 7680, 4320 },
		{ 3840 , 2160 },
		{ 2560 , 1440 },
		{ 1920 , 1080 },
		{ 1600 , 900 },
		{ 1280, 720}
	}};

screenshot::screenshot() : cgv::base::group("Screenshot") {
	framing_ptr = create_and_append_child<framing_overlay>("Framing Editor");
	cgv::signal::connect(framing_ptr->on_change, this, &screenshot::handle_frame_change);
	framing_ptr->gui_options.create_default_tree_node = false;
	framing_ptr->gui_options.show_layout_options = false;

	input_path_prefix =
#ifdef CGV_FORCE_STATIC
		cgv::base::ref_prog_path_prefix();
#else
		"";
#endif

	input_file = cgv::gui::file_helper(this, "Screenshot configuration", cgv::gui::file_helper::Mode::kOpenAndSave);
	input_file.set_title("Open screenshot configuration", cgv::gui::file_helper::Mode::kOpen);
	input_file.set_title("Save screenshot configuration", cgv::gui::file_helper::Mode::kSave);
	input_file.add_filter("Screenshot Configuration", "xml");
	
	input_file.set_default_path(input_path_prefix + "./screenshots.xml", cgv::gui::file_helper::Mode::kOpenAndSave);

	output_directory = cgv::gui::directory_helper(this, "Select output folder", cgv::gui::directory_helper::Mode::kOpen);
	//output_directory.set_directory_name(input_path_prefix + "./screenshots");
	
	help.add_line("Keybindings:");
	help.add_bullet_point("Ctrl + P : Capture all shots");
	help.add_line("");
	help.add_line("Keybindings are only active when the screenshot plugin is selected.");
}

bool screenshot::self_reflect(cgv::reflect::reflection_handler& rh) {
	return rh.reflect_member("screenshots_file", input_file.file_name);
}

bool screenshot::handle(cgv::gui::event& e) {
	// return true if the event gets handled and stopped here or false if you want to pass it to the next plugin
	unsigned et = e.get_kind();

	if(et == cgv::gui::EID_MOUSE) {
		if(active_shot.lock()) {
			if(!edit_view && !edit_frame)
				return true;
		}
	} else if(et == cgv::gui::EID_KEY) {
		cgv::gui::key_event& ke = dynamic_cast<cgv::gui::key_event&>(e);
		if(drawable::active) {
			if(ke.get_action() == cgv::gui::KA_PRESS) {
				if(ke.get_key() == 'P' && ke.get_modifiers() & cgv::gui::EM_CTRL) {
					begin_capture();
					return true;
				}
			}
		}
	}

	return false;
}

void screenshot::on_set(void* member_ptr) {
	const cgv::utils::pointer_test& m(member_ptr);

	if(m.is(input_file.file_name)) {
		const std::string& file_name = input_file.file_name;
		if(input_file.is_save_action()) {
			// force the file name to have a xml extension if not already present
			input_file.ensure_extension("xml", true);

			if(write_shots(file_name)) {
				has_unsaved_changes = false;
				on_set(&has_unsaved_changes);
				update_gui_state();
				auto e = event(EventType::kSaveShots);
				on_change(e);
			} else {
				std::cout << "Error: Could not write shots to " << file_name << std::endl;
			}
		} else {
			bool read = true;

			if(has_unsaved_changes)
				read = cgv::gui::question("Current screenshot file has unsaved changes. Override?") != 0;

			if(read) {
				if(read_shots(file_name)) {
					has_unsaved_changes = false;
					on_set(&has_unsaved_changes);
					std::string directory = cgv::utils::file::get_path(file_name);
					if(!directory.empty())
						output_directory.set_directory_name(directory + "/screenshots");
					update_gui_state();
					auto e = event(EventType::kLoadShots);
					on_change(e);
				} else {
					std::cout << "Error: Could not load shots from " << file_name << std::endl;
				}
			}
		}
	}

	if(active_shot.lock()) {
		bool properties_out_of_date = false;

		if(m.is(edit_view)) {
			if(edit_view) {
				edit_frame = false;
				update_member(&edit_frame);
			} else {
				properties_out_of_date = true;
			}
		}
		if(m.is(edit_frame)) {
			if(edit_frame) {
				properties_out_of_date = true;
				edit_view = false;
				update_member(&edit_view);
			}
		}
		
		if(properties_out_of_date) {
			if(shot_config_ptr shot = active_shot.lock())
				update_shot(shot);
			post_recreate_gui();
		}

		update_framing_overlay();
	}

	if(m.is(resolution_preset)) {
		size_t index = static_cast<size_t>(resolution_preset);
		if(index >= resolution_presets.size())
			index = 0;

		requested_resolution = resolution_presets[index];
		update_member(&requested_resolution.x());
		update_member(&requested_resolution.y());
	}

	if(m.member_of(requested_resolution)) {
		int max_resolution = get_context()->get_device_capabilities().max_render_buffer_size;
		requested_resolution = cgv::math::clamp(requested_resolution, -1, max_resolution);
		update_member(&requested_resolution.x());
		update_member(&requested_resolution.y());
	}

	if(m.is(capture_state.resolution_multiplier))
		update_resolution_info();

	if(m.is(override_gamma))
		update_gui_state();

	update_member(member_ptr);
	post_redraw();
}

bool screenshot::on_exit_request() {
	if(has_unsaved_changes)
		return cgv::gui::question("Screenshot has unsaved changes. Quit?");

	return true;
}

void screenshot::on_select() {
	show();
}

void screenshot::on_deselect() {
	set_active_shot(nullptr, false);

	/*if(shot_config_ptr shot = active_shot.lock()) {
		update_shot(shot);
		set_active_shot(nullptr, false);
	}*/
		
	edit_view = false;
	update_member(&edit_view);
	
	edit_frame = false;
	update_member(&edit_frame);

	hide();
}

void screenshot::init_frame(context& ctx) {
	// Try to read the default screenshots file once if no file name is set.
	if(try_load_default_screenshots_file) {
		try_load_default_screenshots_file = false;
		if(input_file.file_name.empty()) {
			input_file.set_file_name(input_path_prefix + "./screenshots.xml");
			on_set(&input_file.file_name);
		}
	}

	if(last_resolution != get_window_resolution())
		update_resolution_info();
}

void screenshot::after_finish(context& ctx) {
	if(!capture_state.is_capturing)
		return;

	bool show_capture_error_message = false;

	if(init_capture) {
		init_capture = false;
		if(!update_capture_resolution()) {
			show_capture_error_message = true;
			end_capture();
		}
	} else {
		if(await_frame_counter > 0) {
			--await_frame_counter;
		} else {
			bool shot_valid = true;
			std::string shot_name = "screenshot";
			cgv::g2d::irect absolute_frame = {
				cgv::ivec2(0),
				cgv::ivec2(capture_state.capture_resolution)
			};

			if(!capture_state.capture_current_view) {
				if(shot_config_ptr shot = active_shot.lock()) {
					shot_name = (*capture_shot_iterator)->name;
					absolute_frame = {
						cgv::ivec2(shot->frame.position * cgv::vec2(capture_state.capture_resolution)),
						cgv::ivec2(shot->frame.size * cgv::vec2(capture_state.capture_resolution) + 0.5f)
					};
				} else {
					shot_valid = false;
				}
			}

			if(output_directory.directory_name.empty()) {
				end_capture();
				cgv::gui::message("Please specify an output directory.");
			} else if(shot_valid) {
				write_image(output_directory.directory_name + "/" + shot_name + (capture_state.store_transparency ? ".png" : ".bmp"), absolute_frame);
			}

			if(capture_state.capture_current_view) {
				end_capture();
			} else {
				++capture_shot_iterator;
				if(capture_shot_iterator == shots.end()) {
					end_capture();
				} else {
					if(!update_capture_resolution()) {
						show_capture_error_message = true;
						end_capture();
					}
					set_active_shot(*capture_shot_iterator, true);
				}
			}

			/*
			if(capture_single) {
				cgv::g2d::irect absolute_frame = {
						cgv::ivec2(0),
						cgv::ivec2(capture_state.capture_resolution)
				};
				write_image(output_directory.directory_name + "/" + "screenshot" + (capture_state.store_transparency ? ".png" : ".bmp"), absolute_frame);
				end_capture();
			} else {
				if(shot_config_ptr shot = active_shot.lock()) {
					cgv::g2d::irect absolute_frame = {
						cgv::ivec2(shot->frame.position * cgv::vec2(capture_state.capture_resolution)),
						cgv::ivec2(shot->frame.size * cgv::vec2(capture_state.capture_resolution) + 0.5f)
					};
					write_image(output_directory.directory_name + "/" + (*capture_shot_iterator)->name + (capture_state.store_transparency ? ".png" : ".bmp"), absolute_frame);
				}

				++capture_shot_iterator;
				if(capture_shot_iterator == shots.end()) {
					end_capture();
				} else {
					if(!update_capture_resolution()) {
						show_capture_error_message = true;
						end_capture();
					}
					set_active_shot(*capture_shot_iterator, true);
				}
			}
			*/
			await_frame_counter = await_frame_count;
		}
	}

	if(show_capture_error_message)
		cgv::gui::message("Error: Unsupported capture resolution: " + resolution_to_string(capture_state.capture_resolution) + ". Stopping capture.");

	post_redraw();
}

void screenshot::create_gui() {
	add_decorator("Screenshot", "heading", "level=2");
	help.create_gui(this);

	input_file.create_gui("Screenshot file");
	output_directory.create_gui("Output folder", "tooltip='Output folder of captured images'");
	
	add_decorator("", "separator");
	add_decorator("Shots", "heading", "level=4");

	cgv::gui::theme_info& theme = cgv::gui::theme_info::instance();

	size_t shot_index = 0;
	for(shot_config_ptr shot : shots) {
		cgv::gui::property_string options;
		options.add("align", "l");
		options.add("w", 200 - 2 * (12 + 20));

		// highlight selected shot
		if(shot == active_shot.lock()) {
			options.add("color", theme.selection_hex());
			cgv::rgb text_color = cgv::media::inv(theme.text());
			options.add("label_color", cgv::media::to_hex(text_color));
		}

		options.add("tooltip", "Select shot " + shot->name);

		auto shot_param = cgv::signal::const_expression<shot_config_ptr>(shot);
		connect_copy(add_button(shot->name, options, " ")->click, cgv::signal::rebind(this, &screenshot::set_active_shot, cgv::signal::const_expression<shot_config_ptr>(shot), cgv::signal::const_expression<bool>(true), cgv::signal::const_expression<bool>(true)));
		connect_copy(add_button("@<-", "w=20;tooltip='Update user properties'", " ")->click, cgv::signal::rebind(this, &screenshot::update_shot_user_properties, shot_param));
		connect_copy(add_button("@9+", "w=20;tooltip='Delete'")->click, cgv::signal::rebind(this, &screenshot::delete_shot, cgv::signal::const_expression<shot_config_ptr>(shot)));
		++shot_index;
	}

	if(shot_config_ptr shot = active_shot.lock()) {
		const std::string heading_alignment = "\n%y-=10";
		const std::string line_alignment = "\n%y-=10";

		add_decorator("Properties:", "text", "", heading_alignment);
		add_decorator("Create resolution: " + cgv::math::to_string(shot->create_resolution), "text", "", line_alignment);
		add_decorator("Last update resolution: " + cgv::math::to_string(shot->last_update_resolution), "text");

		add_decorator("User properties:", "text", "", heading_alignment);
		size_t i = shot->user_properties.size();
		for(const auto& property : shot->user_properties) {
			add_decorator(property.first + ": " + property.second, "text", "", i > 1 ? line_alignment : "\n");
			--i;
		}

		add_member_control(this, "", rename_shot_name, "string", "tooltip='Specify new name for selected shot'");
		connect_copy(add_button("Rename", "")->click, cgv::signal::rebind(this, &screenshot::rename_shot, cgv::signal::const_expression<shot_config_ptr>(shot)));

		cgv::gui::property_string edit_options;
		edit_options.add("w", (200 - 12) / 2);
		add_member_control(this, "Edit view", edit_view, "toggle", std::string(edit_options) + ";tooltip='Toggle view editing'", " ");
		add_member_control(this, "Edit frame", edit_frame, "toggle", std::string(edit_options) + ";tooltip='Toggle frame editing'");
	}

	add_decorator("New shot", "heading", "level=4");
	add_member_control(this, "Name", new_shot_name, "string", "w=168;tooltip='Specify name or leave empty for default'", " ");
	connect_copy(add_button("@+", "w=20;tooltip='Add new shot'")->click, cgv::signal::rebind(this, &screenshot::create_shot));

	add_decorator("", "separator");
	add_decorator("Capturing", "heading", "level=4");
	add_view("Window resolution", current_resolution, "string");
	add_view("Capture resolution", effective_resolution, "string");
	add_member_control(this, "Resolution multiplier", capture_state.resolution_multiplier, "value", "min=1;max=8;step=1;tooltip='Resolution multiplier applied during capturing'");
	add_member_control(this, "Store transparency", capture_state.store_transparency, "check", "tooltip='Enable to store captured images as PNGs with transparent background'");
	add_member_control(this, "Gamma", capture_state.gamma, "value", "min=0.1;max=5.0;step=0.1;w=124", " ");
	add_member_control(this, "override", override_gamma, "check", "w=64;tooltip='Enable to override the default gamma to the given value'");
	connect_copy(add_button("Capture view", "tooltip='Capture current view'")->click, cgv::signal::rebind(this, &screenshot::capture_view));
	add_decorator("", "separator");
	add_member_control(this, "Use resolution from shot", use_shot_resolution_for_capture, "check", "tooltip='Enable to store captured images as PNGs with transparent background'");
	capture_button = add_button("Capture shots", "tooltip='Capture all shots'");
	connect_copy(capture_button->click, cgv::signal::rebind(this, &screenshot::begin_capture));
	
	
	add_decorator("", "separator");
	add_decorator("Resolution", "heading", "level=4");
	add_decorator("Maximum supported resolution:\n" + resolution_to_string({ get_context()->get_device_capabilities().max_render_buffer_size }), "text");
	std::string resolution_preset_enums = "-," + cgv::utils::transform_join(resolution_presets.begin() + 1, resolution_presets.end(), &screenshot::resolution_to_string, ",");
	add_member_control(this, "Width x Height", requested_resolution.x(), "value", "min=-1;max=7680;step=1;w=78", " ");
	add_member_control(this, "", requested_resolution.y(), "value", "min=-1;max=4320;step=1;w=78", " ");
	add_member_control(this, "", resolution_preset, "dropdown", "w=20;enums='" + resolution_preset_enums + "'");
	connect_copy(add_button("Apply", "tooltip='Resize window to custom resolution'")->click, cgv::signal::rebind(this, &screenshot::apply_requested_resolution));

	update_gui_state();
}

void screenshot::update_gui_state() {
	cgv::gui::theme_info& theme = cgv::gui::theme_info::instance();

	if(auto control = find_view(input_file.file_name))
		control->set("color", has_unsaved_changes ? theme.warning_hex() : theme.control_hex());

	if(capture_button)
		capture_button->set("active", !shots.empty());

	if(auto control = find_control(capture_state.gamma))
		control->set("active", override_gamma);
}

screenshot* screenshot::find_instance(const cgv::base::node* caller) {
	cgv::base::node* node_ptr = const_cast<cgv::base::node*>(caller);
	std::vector<screenshot*> nodes;
	cgv::base::find_interface<screenshot>(cgv::base::base_ptr(node_ptr), nodes);
	if(nodes.empty())
		return nullptr;
	return nodes.front();
}

size_t screenshot::get_shot_count() const {
	return shots.size();
}

std::weak_ptr<const screenshot::shot_config> screenshot::get_shot_at(size_t index) {
	if(index < shots.size()) {
		auto it = shots.begin();
		std::advance(it, index);
		return *it;
	}
	return {};
}

std::weak_ptr<const screenshot::shot_config> screenshot::get_active_shot() const {
	return active_shot;
}

bool screenshot::set_active_shot_by_index(size_t index) {
	if(index < shots.size()) {
		auto it = shots.begin();
		std::advance(it, index);
		set_active_shot(*it, true);
		return true;
	}
	return false;
}

screenshot::capture_info screenshot::get_capture_state() const {
	return capture_state;
}

void screenshot::update_resolution_info() {
	cgv::ivec2 resolution = get_window_resolution();
	current_resolution = resolution_to_string(resolution);
	effective_resolution = resolution_to_string(capture_state.resolution_multiplier * resolution);
	update_member(&current_resolution);
	update_member(&effective_resolution);
}

std::string screenshot::resolution_to_string(cgv::ivec2 resolution) {
	return std::to_string(resolution.x()) + " x " + std::to_string(resolution.y());
}

cgv::uvec2 screenshot::get_window_resolution() const {
	if(context* ctx = get_context())
		return { ctx->get_width(), ctx->get_height() };
	return { 0u, 0u };
}

bool screenshot::set_view_properties(shot_config_ptr shot) {
	view* view_ptr = find_view_as_node();
	if(shot && view_ptr) {
		shot->eye = view_ptr->get_eye();
		shot->focus = view_ptr->get_focus();
		shot->view_up_dir = view_ptr->get_view_up_dir();
		shot->y_view_angle = view_ptr->get_y_view_angle();

		clipped_view* clipped_view_ptr = dynamic_cast<clipped_view*>(view_ptr);
		if(clipped_view_ptr) {
			shot->z_near = clipped_view_ptr->get_z_near();
			shot->z_far = clipped_view_ptr->get_z_far();
		}
		return true;
	}

	return false;
}

void screenshot::set_active_shot(shot_config_ptr shot, bool apply_properties, bool toggle) {
	// do nothing if the shot is already active
	if(shot == active_shot.lock()) {
		if(toggle)
			shot = nullptr;
		else
			return;
	}

	// update view and user properties on previous active shot if view editing is still enabled
	if(edit_view) {
		if(shot_config_ptr shot = active_shot.lock())
			update_shot(shot);
	}

	active_shot = shot;
	edit_view = false;
	edit_frame = false;

	if(shot && apply_properties) {
		view* view_ptr = find_view_as_node();
		if(view_ptr) {
			view_ptr->set_focus(shot->focus);
			view_ptr->set_view_up_dir(shot->view_up_dir);
			view_ptr->set_y_view_angle(shot->y_view_angle);
			view_ptr->set_eye_keep_view_angle(shot->eye);
		}

		clipped_view* clipped_view_ptr = dynamic_cast<clipped_view*>(view_ptr);
		if(clipped_view_ptr) {
			clipped_view_ptr->set_z_near(shot->z_near);
			clipped_view_ptr->set_z_far(shot->z_far);
		}

		// pass shot to callback to allow attached handlers to read user properties
		auto e = event(EventType::kSelectShot, active_shot);
		on_change(e);
	}

	update_framing_overlay();

	post_recreate_gui();
	post_redraw();
}

void screenshot::create_shot() {
	if(!framing_ptr)
		return;

	std::string base_name = cgv::utils::trim(new_shot_name);

	if(base_name.empty())
		base_name = "Shot";

	std::string final_name = base_name;

	size_t test_index = 1;
	while(std::find_if(shots.begin(), shots.end(), [&final_name](const shot_config_ptr shot) { return shot->name == final_name; }) != shots.end()) {
		final_name = base_name + " (" + std::to_string(test_index) + ")";
		++test_index;
	}

	shot_config_ptr shot = std::make_shared<shot_config>();
	shot->name = final_name;
	shot->create_resolution = get_window_resolution();
	shot->last_update_resolution = get_window_resolution();
	shot->frame = { 0.0f, 1.0f }; // the initial frame covers the whole screen
	set_view_properties(shot);
	shots.push_back(shot);

	auto e = event(EventType::kCreateShot, shot);
	on_change(e);

	new_shot_name.clear();
	update_member(&new_shot_name);
	set_active_shot(shot, false);

	has_unsaved_changes = true;
	update_gui_state();
}

void screenshot::rename_shot(shot_config_ptr shot) {
	if(shot) {
		bool valid = !rename_shot_name.empty();
		
		if(std::find_if(shots.begin(), shots.end(), [this](const shot_config_ptr shot) { return shot->name == rename_shot_name; }) != shots.end())
			valid = false;

		if(valid) {
			shot->name = rename_shot_name;
			has_unsaved_changes = true;
			// TODO: Notify handlers that only the name has been updated.
			auto e = event(EventType::kUpdateShot, shot);
			on_change(e);
			rename_shot_name.clear();
			post_recreate_gui();
		}
	}
}

void screenshot::update_shot(shot_config_ptr shot) {
	if(shot) {
		has_unsaved_changes |= set_view_properties(shot);
		shot->last_update_resolution = get_window_resolution();
		has_unsaved_changes |= shot->create_resolution != shot->last_update_resolution;
		
		update_shot_user_properties(shot);
	}
}

void screenshot::update_shot_user_properties(shot_config_ptr shot) {
	if(shot) {
		event e = { EventType::kUpdateShot, shot };
		on_change(e);
		if(e.user_properties_out_of_date_) {
			has_unsaved_changes |= true;
			// recreate the GUI to show the updated values
			post_recreate_gui();
		}

		update_gui_state();
	}
}

void screenshot::delete_shot(shot_config_ptr shot) {
	if(shot && cgv::gui::question("Delete the shot \"" + shot->name + "\"?", "Cancel,Delete", 0)) {
		if(shot == active_shot.lock())
			set_active_shot(nullptr, false);

		shots.remove(shot);
		has_unsaved_changes = true;
		update_gui_state();

		post_recreate_gui();
		post_redraw();
	}
}

void screenshot::apply_requested_resolution() {
	if(auto ctx = get_context()) {
		cgv::uvec2 resolution = get_window_resolution();

		if(requested_resolution.x() > 0)
			resolution.x() = static_cast<unsigned>(requested_resolution.x());

		if(requested_resolution.y() > 0)
			resolution.y() = static_cast<unsigned>(requested_resolution.y());
	
		resolution_preset = cgv::type::DummyEnum(0);
		update_member(&resolution_preset);
		requested_resolution = { -1 };
		update_member(&requested_resolution.x());
		update_member(&requested_resolution.y());

		set_resolution(resolution);
	}
}

bool screenshot::set_resolution(cgv::uvec2 resolution) const {
	if(auto ctx = get_context()) {
		int max_resolution = ctx->get_device_capabilities().max_render_buffer_size;
		if(resolution.x() <= max_resolution && resolution.y() <= max_resolution) {
			ctx->resize(resolution.x(), resolution.y());
			return true;
		}
	}
	return false;
}

bool screenshot::update_capture_resolution() {
	cgv::uvec2 new_resolution = capture_state.base_resolution;
	if(!capture_state.capture_current_view && use_shot_resolution_for_capture && capture_shot_iterator != shots.end())
		new_resolution = (*capture_shot_iterator)->last_update_resolution;

	if(capture_state.resolution_multiplier > 1)
		new_resolution *= static_cast<unsigned>(capture_state.resolution_multiplier);

	if(new_resolution != capture_state.capture_resolution) {
		capture_state.capture_resolution = new_resolution;
		if(!set_resolution(capture_state.capture_resolution))
			return false;
	}
	return true;
}

bool screenshot::setup_capture() {
	cgv::uvec2 resolution = get_window_resolution();
	if(resolution.x() == 0 || resolution.y() == 0)
		return false;

	resolution_backup = resolution;

	capture_state.is_capturing = true;
	await_frame_counter = await_frame_count;
	capture_state.base_resolution = resolution;
	capture_state.capture_resolution = resolution;
	init_capture = true;

	if(override_gamma) {
		if(context* ctx = get_context()) {
			gamma_backup = ctx->get_gamma3();
			ctx->set_gamma(capture_state.gamma);
		}
	}

	if(framing_ptr)
		framing_ptr->set_visibility(false);

	return true;
}

void screenshot::capture_view() {
	if(setup_capture()) {
		capture_state.capture_current_view = true;

		auto e = event(EventType::kBeginCapture);
		on_change(e);
		post_redraw();
	}
}

void screenshot::begin_capture() {
	if(shots.empty())
		return;

	if(setup_capture()) {
		capture_state.capture_current_view = false;
		capture_shot_iterator = shots.begin();

		auto e = event(EventType::kBeginCapture);
		on_change(e);
		set_active_shot(*capture_shot_iterator, true);
	}
}

void screenshot::end_capture() {
	set_resolution(resolution_backup);

	capture_state.is_capturing = false;
	set_active_shot(nullptr, false);

	if(override_gamma) {
		if(context* ctx = get_context())
			ctx->set_gamma3(gamma_backup);
	}

	if(framing_ptr)
		framing_ptr->set_visibility(true);

	auto e = event(EventType::kEndCapture);
	on_change(e);
}

void screenshot::handle_frame_change() {
	if(framing_ptr) {
		if(shot_config_ptr shot = active_shot.lock()) {
			shot->frame = framing_ptr->get_frame();
			has_unsaved_changes = true;
			update_gui_state();
		}
	}
}

void screenshot::update_framing_overlay() {
	if(!framing_ptr)
		return;

	if(shot_config_ptr shot = active_shot.lock())
		framing_ptr->set_frame(shot->frame);
	else
		framing_ptr->clear_frame();

	if(edit_frame)
		framing_ptr->enable_editing();
	else
		framing_ptr->disable_editing();
}

bool screenshot::read_shots(const std::string& file_name) {
	bool success = false;

	set_active_shot(nullptr, false);
	shots.clear();

	const std::string shot_element_name = "Shot";
	const std::string user_property_element_name = "UserProperty";

	tinyxml2::XMLDocument doc;
	if(doc.LoadFile(file_name.c_str()) == tinyxml2::XML_SUCCESS) {
		auto root = doc.RootElement();
		if(root && strcmp(root->Name(), "Screenshots") == 0) {
			const shot_config default_shot;

			auto shot_element = root->FirstChildElement(shot_element_name.c_str());
			while(shot_element) {
				shot_config_ptr shot = std::make_shared<shot_config>();

				bool shot_valid = true;
				if(cgv::xml::QueryStringAttribute(*shot_element, "name", shot->name) != tinyxml2::XML_SUCCESS)
					shot_valid = false;

				shot_valid &= !shot->name.empty();

				if(shot_valid) {
					if(cgv::xml::QueryVecAttribute(*shot_element, "create_resolution", shot->create_resolution) != tinyxml2::XML_SUCCESS)
						shot->create_resolution = { 0u, 0u };

					if(cgv::xml::QueryVecAttribute(*shot_element, "last_update_resolution", shot->last_update_resolution) != tinyxml2::XML_SUCCESS)
						shot->last_update_resolution = { 0u, 0u };

					if(cgv::xml::QueryVecAttribute(*shot_element, "eye", shot->eye) != tinyxml2::XML_SUCCESS)
						shot->eye = default_shot.eye;

					if(cgv::xml::QueryVecAttribute(*shot_element, "focus", shot->focus) != tinyxml2::XML_SUCCESS)
						shot->focus = default_shot.focus;

					if(cgv::xml::QueryVecAttribute(*shot_element, "view_up_dir", shot->view_up_dir) != tinyxml2::XML_SUCCESS)
						shot->view_up_dir = default_shot.view_up_dir;

					if(shot_element->QueryDoubleAttribute("y_view_angle", &shot->y_view_angle) != tinyxml2::XML_SUCCESS)
						shot->y_view_angle = default_shot.y_view_angle;

					if(shot_element->QueryDoubleAttribute("z_near", &shot->z_near) != tinyxml2::XML_SUCCESS)
						shot->z_near = default_shot.z_near;

					if(shot_element->QueryDoubleAttribute("z_far", &shot->z_far) != tinyxml2::XML_SUCCESS)
						shot->z_far = default_shot.z_far;

					cgv::vec4 frame = { 0.0f, 0.0f, 1.0f, 1.0f };
					if(cgv::xml::QueryVecAttribute(*shot_element, "frame", frame) == tinyxml2::XML_SUCCESS) {
						shot->frame.x() = frame.x();
						shot->frame.y() = frame.y();
						shot->frame.w() = frame.z();
						shot->frame.h() = frame.w();
					} else {
						shot->frame = default_shot.frame;
					}

					// Read user properties
					auto user_property = shot_element->FirstChildElement(user_property_element_name.c_str());
					while(user_property) {
						auto attribute = user_property->FirstAttribute();
						if(attribute) {
							std::string key, value;
							key = attribute->Name();
							value = attribute->Value();
							if(!key.empty())
								shot->user_properties.insert({ key, value });
						}

						user_property = user_property->NextSiblingElement(user_property_element_name.c_str());
					}

					shots.push_back(shot);
				}

				shot_element = shot_element->NextSiblingElement(shot_element_name.c_str());
			}

			success = true;
		}
	}

	post_recreate_gui();
	post_redraw();
	return success;
}

bool screenshot::write_shots(const std::string& file_name) const {
	tinyxml2::XMLDocument doc;

	auto root = doc.NewElement("Screenshots");
	doc.InsertFirstChild(root);

	for(shot_config_ptr shot : shots) {
		auto shot_element = doc.NewElement("Shot");
		root->InsertEndChild(shot_element);

		shot_element->SetAttribute("name", shot->name.c_str());
		shot_element->SetAttribute("create_resolution", cgv::math::to_string(shot->create_resolution).c_str());
		shot_element->SetAttribute("last_update_resolution", cgv::math::to_string(shot->last_update_resolution).c_str());
		shot_element->SetAttribute("eye", cgv::math::to_string(shot->eye).c_str());
		shot_element->SetAttribute("focus", cgv::math::to_string(shot->focus).c_str());
		shot_element->SetAttribute("view_up_dir", cgv::math::to_string(shot->view_up_dir).c_str());
		shot_element->SetAttribute("y_view_angle", shot->y_view_angle);
		shot_element->SetAttribute("z_near", shot->z_near);
		shot_element->SetAttribute("z_far", shot->z_far);
		cgv::vec4 frame = {
			shot->frame.x(),
			shot->frame.y(),
			shot->frame.w(),
			shot->frame.h()
		};
		shot_element->SetAttribute("frame", cgv::math::to_string(frame).c_str());

		for(const auto& entry : shot->user_properties) {
			auto user_property = doc.NewElement("UserProperty");
			user_property->SetAttribute(entry.first.c_str(), entry.second.c_str());
			shot_element->InsertEndChild(user_property);
		}
	}

	return doc.SaveFile(file_name.c_str()) == tinyxml2::XML_SUCCESS;
}

void screenshot::write_image(const std::string& file_name, const cgv::g2d::irect& rectangle) {
	context* ctx = get_context();
	if(!ctx) {
		std::cout << "Error: screenshot::write_image could not aquire context" << std::endl;
		return;
	}

	std::string folder_path = cgv::utils::file::get_path(file_name);
	if(!cgv::utils::dir::exists(folder_path))
		cgv::utils::dir::mkdir(folder_path);

	std::cout << "Writing " << file_name << " ...";
	if(ctx->write_frame_buffer_to_image(file_name, capture_state.store_transparency ? cgv::data::CF_RGBA : cgv::data::CF_RGB, FB_FRONT, rectangle.x(), rectangle.y(), rectangle.w(), rectangle.h()))
		std::cout << "OK" << std::endl;
	else
		std::cout << "Error" << std::endl;
}

#include <cgv/base/register.h>
cgv::base::object_registration<screenshot> screenshot_reg("");
