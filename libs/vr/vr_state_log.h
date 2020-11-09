#pragma once
#include <cgv/render/render_types.h>
#include <vector>
#include <unordered_map>

#include <libs/vr/vr_state.h>
#include "vr_driver.h"

namespace vr {
	//! helper struct for logging vr events
	template <template <typename, typename> class cont>
	struct vr_state_log : public cgv::render::render_types {
		template<class T>
		using container = cont<T, std::allocator<T>>;

		enum ProtocolMode {
			PM_IN_MEMORY = 1,
			PM_OSTREAM = 2,
			PM_IN_MEMORY_AN_OSTREAM = 3,
			PM_NONE = 0
		};

		enum Filter {
			F_POSE = 1,
			F_BUTTON = 2,
			F_THROTTLE = 4,
			F_VIBRATION = 8,
			F_HMD = 16,
			F_ALL = 31
		};

	private:
		rgb default_controller_arrow_color = { 1.f, 0.f, 0.f };
		int prev_pose_index[2] = { -1,-1 };
		int last_pose_index[2] = { -1,-1 };
		/// accumulates error for rejected arrows which were too similar to the previous added arrow
		float last_arrow_accumulated_path_energy[2] = { 0.f, 0.f };

		bool create_render_helpers = false;

		std::ostream log_stream;
		int protocol_mode = PM_NONE;
	public:
		float controller_hue[2] = { 1.0f, 0.68f };
		double controller_arrows_min_energy = 0.01;
		double controller_arrows_rotation_weighting = 2.0;
		//double controller_arrows_translation_weighting = 1.0;

		//render helper vectors
		std::vector<rgb> trajectory_cone_color;
		std::vector<float> trajectory_cone_radii;
		std::vector<vec3> trajectory_cone_positions;

		std::vector<vec3> arrow_controller_positions;
		std::vector<vec3> arrow_controller_directions;
		std::vector<rgba> arrow_colors;

		//hmd state
		container<float[12]> hmd_pose;
		container<double> hmd_time_stamp;
		//controller states
		container<double> controller_time_stamp;
		container<int8_t> controller_id;
		container<float[8]> controller_axes;
		container<float[12]> controller_pose;
		container<float[2]> controller_vibration;
		container<unsigned> controller_button_flags;

		/** adds render information for a pose based on contents of trajectory_* vectors
		*  @param[in] pose_ix pose index
		*  @param[in] prev_index index of the previous pose. Used for connecting points
		*/
		/*
		void add_pose_render_information(int pose_ix, int prev_index) {
			unsigned cid = event_cid[trajectory_event_ids[pose_ix]];
			assert(static_cast<unsigned>(cid) < 2 && pose_ix >= 0);
			cgv::media::color<float, cgv::media::HLS> hls_color(controller_hue[cid], 0.5f, 0.8f);

			if (prev_index >= 0) {
				trajectory_cone_positions.push_back(trajectory_translation[prev_index]);
				trajectory_cone_positions.push_back(trajectory_translation[pose_ix]);
				trajectory_cone_radii.push_back(0.004f);
				trajectory_cone_radii.push_back(0.004f);
				trajectory_cone_color.emplace_back(hls_color);
				trajectory_cone_color.emplace_back(hls_color);
			}

			add_controller_arrow(trajectory_translation[pose_ix], trajectory_orientation[pose_ix].apply(vec3(0, 0, -1.f)), cid, prev_index, pose_ix, controller_arrows_min_energy);
		}
		*/

		/** difference between previous and next point, the arrow can be rejected if the energy dependency is not fullfilled
		*   @param[in] direction The direction in which the arrow points
		*   @param[in] min_energy Minimal acceptable difference between arrows
		*/
		/*
		bool add_controller_arrow(const vec3& position, const vec3& direction, const int ci, const int prev_pose_index, const int pose_index, const float min_energy) {
			enum {
				NO_PREVIOUS,
				USE_ENERGY
			};
			int mode = (prev_pose_index < 0) ? NO_PREVIOUS : USE_ENERGY;

			switch (mode) {
			case USE_ENERGY:
			{
				quat diff = trajectory_orientation[prev_pose_index].inverse() * trajectory_orientation[pose_index];

				// add length and rotational difference (cos)
				last_arrow_accumulated_path_energy[ci] += (trajectory_translation[prev_pose_index] - position).length()
					+ (controller_arrows_min_energy*controller_arrows_rotation_weighting*(1.0/3.14159265358979323846))*(2.f * acosf(diff.w()));
				if (last_arrow_accumulated_path_energy[ci] < min_energy)
					return false;
			}
			case NO_PREVIOUS:
				arrow_controller_directions.emplace_back(direction);
				arrow_colors.emplace_back(default_controller_arrow_color);
				arrow_controller_positions.emplace_back(position);
				last_arrow_accumulated_path_energy[ci] = 0;
				return true;
			}
		}
		*/
		/** replaces cone color render information with recalculated colors based on controller_hue*/
		/*
		void update_trajectory_color_render_information() {
			trajectory_cone_color.clear();
			for (int i = 0; i < trajectory_time_stamps.size(); ++i) {
				int cid = event_cid[trajectory_event_ids[i]];
				if (cid >= 0 && cid < 2) {
					cgv::media::color<float, cgv::media::HLS> hls_color(controller_hue[cid], 0.5f, 0.8f);
					trajectory_cone_color.emplace_back(hls_color);
					trajectory_cone_color.emplace_back(hls_color);
				}
			}
		}*/

		/** redoes the arrow position calculation*/
		/*
		void update_arrow_render_information() {
			//need to replay protocol
			int prev_pose_index[2] = { -1,-1 };
			int last_pose_index[2] = { -1,-1 };

			arrow_colors.clear();
			arrow_controller_directions.clear();
			arrow_controller_positions.clear();
			last_arrow_accumulated_path_energy[0] = last_arrow_accumulated_path_energy[1] = 0;

			for (int pose_ix = 0; pose_ix < trajectory_time_stamps.size(); ++pose_ix) {
				unsigned ci = event_cid[trajectory_event_ids[pose_ix]];
				add_controller_arrow(trajectory_translation[pose_ix], trajectory_orientation[pose_ix].apply(vec3(0, 0, -1.f)), ci, prev_pose_index[ci], pose_ix, controller_arrows_min_energy);
				prev_pose_index[ci] = last_pose_index[ci];
				last_pose_index[ci] = pose_ix;
			}
		}*/

		void enable_in_memory_log() {
			protocol_mode = protocol_mode | PM_IN_MEMORY;
		}
		void enable_ostream_log(std::ostream&& os) {
			log_stream = os;
		}

		void log_vr_state(vr::vr_kit_state& state, int mode, int filter, double time) {
			for (int i = 0; i < 4; ++i) {
				if (state.controller[i].status == vr::VRS_TRACKED) {
					if (mode && PM_IN_MEMORY) {
						this->controller_time_stamp.push_back(time);
						this->controller_id.push_back(i);
						if (filter && F_VIBRATION) {
							this->controller_vibration.push_back(state.controller[i].vibration);
						}
						if (filter && F_THROTTLE) {
							this->controller_axes.push_back(state.controller[i].axes);
						}
						if (filter && F_POSE) {
							this->controller_pose.push_back(state.controller[i].pose);
						}
						if (filter && F_BUTTON) {
							this->controller_button_flags.push_back(state.controller[i].button_flags);
						}
					}
					if (mode && PM_OSTREAM) {
						//C <timestamp> <controller_id> [P <pose>] [B <button-mask>] [T <throttle-state>] [V <vibration>]
						*(os) << "C " << time << ' ' << i;
						if (filter && F_POSE) {
							log_stream << " P " << state.controller[i].pose;
						}
						if (filter && F_BUTTON) {
							log_stream << " B " << state.controller[i].button_flags;
						}
						if (filter && F_THROTTLE) {
							log_stream << " T " << state.controller[i].axes;
						}
						if (filter && F_VIBRATION) {
							log_stream << " V " << state.controller[i].vibration;
						}
						log_stream << "\n";
					}
				}
			}

			if (state.hmd.status == VRS_TRACKED) {
				if (mode && PM_IN_MEMORY) {
					hmd_time_stamp.push_back(time);
					hmd_pose.push_back(state.hmd.pose);
				}
				if (mode && PM_OSTREAM) {
					log_stream << "H " << time << " " << state.hmd.pose;
				}
			}
		}
		/*
		bool read_log(std::istringstream f) {
			std::string line;
			int line_number = 0;

			int prev_pose_index[2] = { -1,-1 };
			int last_pose_index[2] = { -1,-1 };

			try {
				while (!f.eof()) {
					std::getline(f, line); 	//read a line
					std::istringstream l(line);
					int32_t controller_id = -1;
					double time_in_ms;
					std::string event_type;
					++line_number;

					l >> time_in_ms;
					l >> controller_id;
					l >> event_type;
					if (l.eof()) {
						return true;
					}
					if (event_type == "vr_pose") {
						quat orientation;
						vec3 position;
						l >> position;
						l >> orientation;
						int pose_ix = trajectory_translation.size();
						prev_pose_index[controller_id] = last_pose_index[controller_id];
						last_pose_index[controller_id] = pose_ix;
						trajectory_translation.push_back(std::move(position));
						trajectory_orientation.push_back(std::move(orientation));
						trajectory_time_stamps.push_back(time_in_ms);
						trajectory_event_ids.push_back(event_cid.size());
						event_cid.push_back(controller_id);
						if (create_render_helpers)
							add_pose_render_information(pose_ix, prev_pose_index[controller_id]);
					}
					else if (event_type == "vr_stick") {
						vec2 touch;
						cgv::gui::StickAction action;
						std::string as;
						l >> as;
						action = stick_action_from(as);
						l >> touch.x();
						l >> touch.y();
						stick_time_stamps.push_back(time_in_ms);
						stick_actions.push_back(action);
						stick_positions.push_back(touch);
						stick_event_ids.push_back(event_cid.size());
						event_cid.push_back(controller_id);
					}
					else if (event_type == "vr_key") {
						unsigned short key;
						cgv::gui::KeyAction key_action;
						std::string key_str, key_action_str;
						l >> key_str;
						l >> key_action_str;
						key = key_from(key_str);
						key_action = key_action_from(key_action_str);
						key_event_timestamps.push_back(time_in_ms);
						key_event_keys.push_back(key);
						key_event_actions.push_back(key_action);
						key_event_event_ids.push_back(event_cid.size());
						event_cid.push_back(controller_id);
					}
					else if (event_type == "vr_throttle") {
						float value;
						l >> value;
						throttle_timestamps.push_back(time_in_ms);
						event_throttle_state.push_back(value);
						throttle_event_ids.push_back(event_cid.size());
						event_cid.push_back(controller_id);
					}
					else {
						std::cout << "vr_kit_log::load_log: ignoring unknown event type: " << event_type << " on line " << line_number << "\n";
					}

				}
			}
			catch (std::string err) {
				std::cerr << err << '\n' << "vr_kit_log::load_log: failed parsing data from file\n";
				return false;
			}

			return true;
		}
		*/
		void clear_events() noexcept {
			last_pose_index[0] = last_pose_index[1] = prev_pose_index[0] = prev_pose_index[1] = -1;
			last_arrow_accumulated_path_energy[0] = last_arrow_accumulated_path_energy[1] = 0.f;

			trajectory_cone_radii.clear();
			trajectory_cone_color.clear();
			trajectory_cone_positions.clear();
			arrow_controller_positions.clear();
			arrow_controller_directions.clear();
			arrow_colors.clear();
			states.clear();
			time_stamp.clear();
		}

	protected:
		cgv::gui::StickAction stick_action_from(const std::string& name) {
			auto it = to_stick_action.find(name);
			if (it != to_stick_action.end()) {
				return it->second;
			}
			throw std::string("exception vr_kit_log stick_action_from: unknown name");
		}

		unsigned short key_from(const std::string& name) noexcept {
			auto it = to_key.find(name);
			if (it != to_key.end()) {
				return it->second;
			}
			return vr::VR_UNKNOWN;
		}

		cgv::gui::KeyAction key_action_from(const std::string& name) {
			auto it = to_key_action.find(name);
			if (it != to_key_action.end()) {
				return it->second;
			}
			throw std::string("exception vr_kit_log key_action_from: unknown name");
		}

	private:

		static const std::unordered_map<std::string, cgv::gui::StickAction> to_stick_action;
		static const std::unordered_map<std::string, unsigned short> to_key;
		static const std::unordered_map<std::string, cgv::gui::KeyAction> to_key_action;
	};
	/*
	template <template <typename, typename> class cont>
	const std::unordered_map<std::string, cgv::gui::StickAction>  vr_kit_log<cont>::to_stick_action = {
	   {"touch", cgv::gui::StickAction::SA_TOUCH},
	   {"press", cgv::gui::StickAction::SA_PRESS},
	   {"unpress", cgv::gui::StickAction::SA_UNPRESS},
	   {"release", cgv::gui::StickAction::SA_RELEASE},
	   {"move", cgv::gui::StickAction::SA_MOVE},
	   {"drag", cgv::gui::StickAction::SA_DRAG}
	};
	template <template <typename, typename> class cont>
	const std::unordered_map<std::string, unsigned short>  vr_kit_log<cont>::to_key = {
	   {"VR_UNKNOWN", vr::VR_UNKNOWN},
	   {"VR_SYSTEM", vr::VR_SYSTEM},
	   {"VR_MENU", vr::VR_MENU},
	   {"VR_GRIP", vr::VR_GRIP},
	   {"VR_DPAD_DOWN_LEFT", vr::VR_DPAD_DOWN_LEFT},
	   {"VR_DPAD_DOWN", vr::VR_DPAD_DOWN},
	   {"VR_DPAD_DOWN_RIGHT", vr::VR_DPAD_DOWN_RIGHT},
	   {"VR_DPAD_LEFT", vr::VR_DPAD_LEFT},
	   {"VR_DPAD_RIGHT", vr::VR_DPAD_RIGHT},
	   {"VR_DPAD_UP_LEFT", vr::VR_DPAD_UP_LEFT},
	   {"VR_DPAD_UP", vr::VR_DPAD_UP},
	   {"VR_DPAD_UP_RIGHT", vr::VR_DPAD_UP_RIGHT},
	   {"VR_A", vr::VR_A},
	   {"VR_INPUT0_TOUCH", vr::VR_INPUT0_TOUCH},
	   {"VR_INPUT0", vr::VR_INPUT0},
	   {"VR_INPUT1_TOUCH", vr::VR_INPUT1_TOUCH},
	   {"VR_INPUT1", vr::VR_INPUT1},
	   {"VR_INPUT2_TOUCH", vr::VR_INPUT2_TOUCH},
	   {"VR_INPUT2", vr::VR_INPUT2},
	   {"VR_INPUT3_TOUCH", vr::VR_INPUT3_TOUCH},
	   {"VR_INPUT3", vr::VR_INPUT3},
	   {"VR_INPUT4_TOUCH", vr::VR_INPUT4_TOUCH},
	   {"VR_INPUT4", vr::VR_INPUT4},
	   {"VR_PROXIMITY", vr::VR_PROXIMITY}
	};

	template <template <typename, typename> class cont>
	const std::unordered_map<std::string, cgv::gui::KeyAction>  vr_kit_log<cont>::to_key_action = {
	   {"press", cgv::gui::KA_RELEASE},
	   {"release", cgv::gui::KA_PRESS},
	   {"repeat", cgv::gui::KA_REPEAT}
	};
	*/
}
