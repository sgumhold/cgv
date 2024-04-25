#pragma once
#include <vector>
#include <unordered_map>
#include <ostream>
#include <sstream>
#include <memory>

#include <libs/vr/vr_state.h>
#include <cgv/data/ref_counted.h>
//#include <cgv/math/fvec.h>
//#include <cgv/math/vec.h>
#include <cgv/math/fmat.h>
#include "vr_driver.h"

#include "lib_begin.h"

namespace vr {
	//! helper struct for logging vr events
	class CGV_API vr_log : public cgv::data::ref_counted {
	public:
		template<class T>
		using container = std::vector<T, std::allocator<T>>;
		using vec2 = cgv::vec2;
		using vecn = cgv::vecn;
		using mat34 = cgv::mat34;
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
			F_ALL = 31,
			F_NONE = 0
		};

		container<double> time_stamp;

		container<mat34> hmd_pose;
		container<uint8_t> hmd_status;

		container<vecn> controller_axes[max_nr_controllers];
		container<mat34> controller_pose[max_nr_controllers];
		container<vec2> controller_vibration[max_nr_controllers];
		container<unsigned> controller_button_flags[max_nr_controllers];
		container<uint8_t> controller_status[max_nr_controllers];
	private:
		bool setting_locked = false;
		int log_storage_mode = SM_NONE;
		int filters = 0;
		size_t nr_vr_states = 0; //number of recorded vr states

		std::shared_ptr<std::ostream> log_stream;

		inline void unlock_settings() {
			setting_locked = false;
		}
	protected:
		//! record state
		void log_vr_state(const vr::vr_kit_state& state, const int mode, const int filter, const double time, std::ostream* log_stream);
	public:
		vr_log() = default;
		//construct log from stream
		vr_log(std::istringstream& is);

		//! write vr_kit_state to log , and stream serialized vr_kit_state to log_stream if ostream_log is enabled
		inline void log_vr_state(const vr::vr_kit_state& state, const double& time) {
			log_vr_state(state, log_storage_mode, filters, time, log_stream.get());
		}
		//! disable logging
		void disable_log();
		//! enable in memory log
		void enable_in_memory_log();

		//! enable writing to ostream.
		void enable_ostream_log(const std::shared_ptr<std::ostream>& stream);

		//! define what data should be recorded.
		inline void set_filter(int f) {
			if (setting_locked)
				return;
			filters = f;
		}
		inline int get_filter() const {
			return filters;
		}

		//! prevent changes to settings and enables log_vr_state methods
		void lock_settings();

		inline const size_t recorded_vr_states() const {
			return nr_vr_states;
		}

		//! read log from stream
		bool load_state(std::istringstream& is);
	};
}

#include <cgv/config/lib_end.h>