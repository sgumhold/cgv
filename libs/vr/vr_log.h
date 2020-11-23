#pragma once
#include <cgv/render/render_types.h>
#include <vector>
#include <unordered_map>
#include <ostream>
#include <sstream>

#include <libs/vr/vr_state.h>
#include "vr_driver.h"

#include "lib_begin.h"

namespace vr {
	//! helper struct for logging vr events
	class CGV_API vr_log : public cgv::render::render_types {
	public:
		template<class T>
		using container = std::vector<T, std::allocator<T>>;
		using vec8 = cgv::math::fvec<float, 8>;
		enum StorageMode {
			SM_IN_MEMORY = 1,
			SM_OSTREAM = 2,
			SM_IN_MEMORY_AND_OSTREAM = 3,
			SM_NONE = 0
		};

		enum Filter {
			F_POSE = 1,
			F_BUTTON = 2,
			F_AXES = 4,
			F_VIBRATION = 8,
			F_HMD = 16,
			F_ALL = 31
		};

		//hmd state
		container<mat34> hmd_pose;
		container<double> time_stamp;
		//container<double> hmd_time_stamp;
		//controller states
		
		//container<double> controller_time_stamp;
		//container<int8_t> controller_id;
		container<vec8> controller_axes[4];
		container<mat34> controller_pose[4];
		container<vec2> controller_vibration[4];
		container<unsigned> controller_button_flags[4];
		container<uint8_t> controller_status[4];
		container<uint8_t> hmd_status;

	private:
		bool setting_locked = false;
		int log_storage_mode = SM_NONE;
		int filters = 0;

		container<std::pair<unsigned, unsigned>> vr_states_ix;
		unsigned vr_state_num = 0;

		inline void unlock_settings() {
			setting_locked = false;
		}
	protected:
		//! record state
		void log_vr_state(const vr::vr_kit_state& state, const int mode, const int filter, const double time, std::ostream* log_stream);
		//! read log from stream
		void load_state(std::istringstream& is, const char terminator = '\0');
	public:
		vr_log() = default;
		//construct log from stream
		vr_log(std::istringstream& is, const char terminator='\0');

		//! write vr_kit_state to log , and stream serialized vr_kit_state to log_stream if ostream_log is enabled
		inline void log_vr_state(const vr::vr_kit_state& state, const double& time, std::ostream* log_stream = nullptr) {
			log_vr_state(state, log_storage_mode, filters, time, log_stream);
		}
		//! disable logging
		void disable_log();
		//! enable in memory log
		inline void enable_in_memory_log() {
			if (!setting_locked)
				log_storage_mode = log_storage_mode | SM_IN_MEMORY;
		}

		//! enable writing to ostream.
		inline void enable_ostream_log() {
			if (!setting_locked)
				log_storage_mode = log_storage_mode | SM_OSTREAM;
		}
		inline void set_filter(int f) {
			if (setting_locked)
				return;
			filters = f;
		}
		//! prevent changes to settings and enables log_vr_state methods
		inline void lock_settings() {
			setting_locked = true;
		}

		 
	};
}

#include <cgv/config/lib_end.h>