#include "vr_log.h"

/* Logfile lines
<pose> : 12 floats representing a 4*3 matrix (column major)
<button-mask>: 32 bit integer
<axis-state>: 8 floats for the axes of the controller
<vibration>: 2 floats for the vibration intensity
<timestamp>: time in ms

<timestamp> (C <controller_id> [P <pose>] [B <button-mask>] [A <axes-state>] [V <vibration>])* [H <pose>]
*/


void vr::vr_log::disable_log()
{
	log_storage_mode = SM_NONE;
}

vr::vr_log::vr_log(std::istringstream& is, char terminator) {
	load_state(is, terminator);
}

void vr::vr_log::log_vr_state(const vr::vr_kit_state& state, const int mode, const int filter, const double time,std::ostream* log_stream)
{
	if (!setting_locked)
		return;
	++nr_vr_states;
	//time stamp
	if (mode & SM_IN_MEMORY) {
		this->time_stamp.push_back(time);
	}
	if (mode & SM_OSTREAM) {
		*(log_stream) << time;
	}

	//controller state
	for (int i = 0; i < 4; ++i) {
		controller_status->push_back(state.controller[i].status);
		if (mode & SM_IN_MEMORY) {
			if (filter & F_VIBRATION) {
				vec2 vibration = vec2(state.controller[i].vibration[0], state.controller[i].vibration[1]);
				this->controller_vibration[i].push_back(vibration);
			}
			if (filter & F_AXES) {
				vec8 axes;
				for (int i = 0; i < 8; ++i) {
					axes(i) = state.controller[i].axes[i];
				}
				this->controller_axes[i].push_back(axes);
			}
			if (filter & F_POSE) {
				mat34 pose = mat34(3, 4, state.controller[i].pose);
				this->controller_pose[i].push_back(pose);
			}
			if (filter & F_BUTTON) {
				this->controller_button_flags[i].push_back(state.controller[i].button_flags);
			}
		}
		if (mode & SM_OSTREAM) {
			//C <timestamp> <controller_id> [P <pose>] [B <button-mask>] [T <throttle-state>] [V <vibration>]
			*(log_stream) << " C " << i;
			if (filter & F_POSE) {
				*(log_stream) << " P";
				for (int j = 0; j < 12; ++j)
					*(log_stream) << ' ' << state.controller[i].pose[j];
			}
			if (filter & F_BUTTON) {
				*(log_stream) << " B " << state.controller[i].button_flags;
			}
			if (filter & F_AXES) {
				*(log_stream) << " A";
				for (int j = 0; j < 8; ++j)
					*(log_stream) << ' ' << state.controller[i].axes[j];
			}
			if (filter & F_VIBRATION) {
				*(log_stream) << " V";
				for (int j = 0; j < 2; ++j)
					*(log_stream) << ' ' << state.controller[i].vibration[j];
			}
		}
	}

	//hmd state
	if (filter & F_HMD) {
		mat34 pose = mat34(3, 4, state.hmd.pose);
		if (mode & SM_IN_MEMORY) {
			hmd_pose.push_back(pose);
			hmd_status.push_back(state.hmd.status);
		}
		if ((mode & SM_OSTREAM) && log_stream) {
			*(log_stream) << "H " << time;
			for (int j = 0; j < 12; ++j)
				*(log_stream) << ' ' << state.hmd.pose[j];
		}
	}
	//end line
	if ((mode & SM_OSTREAM) && log_stream) {
		*(log_stream) << '\n';
	}
}

bool vr::vr_log::load_state(std::istringstream& is, const char terminator) {
	//not implemented
	assert(false);
	return false;
}