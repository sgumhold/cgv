#pragma once

#include <memory>
#include <list>
#include <cgv/base/group.h>
#include <cgv/gui/directory_helper.h>
#include <cgv/gui/event_handler.h>
#include <cgv/gui/file_helper.h>
#include <cgv/gui/help_message.h>
#include <cgv/gui/provider.h>
#include <cgv/render/drawable.h>

#include "framing_overlay.h"

#include "lib_begin.h"

class CGV_API screenshot :
	public cgv::base::group,
	public cgv::render::drawable,
	public cgv::gui::provider,
	public cgv::gui::event_handler {
public:
	using property_map = std::map<std::string, std::string>;

	class shot_config {
	public:
		std::string name;
		cgv::uvec2 create_resolution = 0;
		cgv::uvec2 last_update_resolution = 0;
		cgv::g2d::rect frame;
		cgv::dvec3 eye = { 0.0, 0.0, 5.0 };
		cgv::dvec3 focus = { 0.0, 0.0, 0.0 };
		cgv::dvec3 view_up_dir = { 0.0, 1.0, 0.0 };
		double y_view_angle = 0.0;
		double z_near = 0.0001;
		double z_far = 100.0;
		property_map user_properties;

		std::string get_user_property_value(const std::string& key) const {
			auto it = user_properties.find(key);
			if(it != user_properties.end())
				return it->second;
			throw std::runtime_error{ "shot_config::user_properties: could not find key " + key};
		}
	};

	using shot_config_ptr = std::shared_ptr<shot_config>;
	using shot_config_cptr = std::shared_ptr<const shot_config>;

	struct capture_info {
		/// whether the capture process is active
		bool is_capturing = false;
		/// the used base resolution
		cgv::uvec2 base_resolution = { 0 };
		/// the used capture resolution (base_resolution * resolution multiplier)
		cgv::uvec2 capture_resolution = { 0 };
		/// the used resolution multiplier
		int resolution_multiplier = 1;
		/// whether to store transparency
		bool store_transparency = false;
		/// the gamma value used during capturing
		float gamma = 2.2f;
	};

	enum class EventType {
		kUndefined,
		kLoadShots,
		kSaveShots,
		kCreateShot,
		kUpdateShot,
		kRemoveShot,
		kSelectShot,
		kBeginCapture,
		kEndCapture
	};

	class event {
	public:
		event(EventType type) : type_(type) {}
		event(EventType type, std::weak_ptr<shot_config> shot) : type_(type), shot_(shot) {}

		EventType get_type() const { return type_; }

		std::weak_ptr<const shot_config> get_shot() const {
			return shot_;
		};

		bool set_shot_user_properties(const property_map& properties) {
			if(auto shot = shot_.lock()) {
				shot->user_properties = properties;
				user_properties_out_of_date_ = true;
				return true;
			}
			return false;
		};

	private:
		friend class screenshot;

		EventType type_ = EventType::kUndefined;
		std::weak_ptr<shot_config> shot_;
		bool user_properties_out_of_date_ = false;
	};

	screenshot();
	std::string get_type_name() const { return "screenshot"; }

	bool self_reflect(cgv::reflect::reflection_handler& rh) override;
	void stream_help(std::ostream& os) override {}

	bool handle(cgv::gui::event& e) override;
	void on_set(void* member_ptr) override;
	bool on_exit_request() override;

	void on_select() override;
	void on_deselect() override;

	void init_frame(cgv::render::context& ctx) override;
	void after_finish(cgv::render::context& ctx) override;

	void create_gui() override;

	static screenshot* find_instance(const cgv::base::node* caller);

	size_t get_shot_count() const;

	std::weak_ptr<const shot_config> get_shot_at(size_t index);

	std::weak_ptr<const shot_config> get_active_shot() const;

	bool set_active_shot_by_index(size_t index);

	capture_info get_capture_state() const;

	cgv::signal::signal<event&> on_change;

private:
	static const std::array<cgv::ivec2, 7> resolution_presets;

	cgv::gui::help_message help;

	bool try_load_default_screenshots_file = true;
	std::string input_path_prefix;
	cgv::gui::file_helper input_file;
	cgv::gui::directory_helper output_directory;

	framing_overlay_ptr framing_ptr;

	std::list<shot_config_ptr> shots;
	std::weak_ptr<shot_config> active_shot;
	std::string new_shot_name;
	std::string rename_shot_name;

	bool has_unsaved_changes = false;
	bool edit_view = false;
	bool edit_frame = false;

	cgv::ivec2 last_resolution = { -1 };
	std::string current_resolution;
	std::string effective_resolution;

	cgv::vec3 gamma_backup = { 1.0f };
	cgv::uvec2 resolution_backup;

	capture_info capture_state;
	bool capture_single = false;
	bool init_capture = true;
	const int await_frame_count = 3;
	int await_frame_counter = await_frame_count;
	bool override_gamma = false;
	bool use_shot_resolution_for_capture = false;
	std::list<shot_config_ptr>::const_iterator capture_shot_iterator;
	cgv::gui::button_ptr capture_button;

	cgv::type::DummyEnum resolution_preset = cgv::type::DummyEnum(0);
	cgv::ivec2 requested_resolution = { -1 };

	void update_gui_state();

	void update_resolution_info();

	static std::string resolution_to_string(cgv::ivec2 resolution);

	cgv::uvec2 get_window_resolution() const;

	bool set_view_properties(shot_config_ptr shot);

	void set_active_shot(shot_config_ptr shot, bool apply_properties, bool toggle = false);

	void create_shot();

	void rename_shot(shot_config_ptr shot);

	void update_shot(shot_config_ptr shot);

	void update_shot_user_properties(shot_config_ptr shot);

	void delete_shot(shot_config_ptr shot);

	void apply_requested_resolution();

	bool set_resolution(cgv::uvec2 resolution) const;

	bool update_capture_resolution();

	bool setup_capture();

	void capture_view();

	void begin_capture();

	void end_capture();

	void handle_frame_change();

	void update_framing_overlay();

	bool read_shots(const std::string& file_name);

	bool write_shots(const std::string& file_name) const;

	void write_image(const std::string& file_name, const cgv::g2d::irect& rectangle);
};

#include <cgv/config/lib_end.h>
