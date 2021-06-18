#include "vr_log.h"

#include <sstream>

/* Logfile lines
<pose> : 12 floats representing a 4*3 matrix (column major)
<button-mask>: 32 bit integer
<axis-state>: vr::max_nr_controller_axes floats for the axes of the controller
<vibration>: 2 floats for the vibration intensity
<timestamp>: time in ms

<timestamp>,({C <controller_id> [P <pose>] [B <button-mask>] [A <axes-state>] [V <vibration>]},)*[H <pose>]
*/

//token struct for the parser
struct token {
	enum {
		COMPOUND, //<name> <...>
		VALUE,// <value>
		NAME
	} type;
	std::string text;

};

template <size_t size, typename T>
bool array_contains(const T* container, const T& element) {
	int i = 0;
	while (i < size) {
		if (container[i] == element) {
			return true;
		}
		++i;
	}
	return false;
}


template <typename C>
void tokenize(std::string& line, C& buffer) {
	std::istringstream l(line);
	std::string text;
	while (std::getline(l, text, ',')) {
		if (text.size()>0 && text.c_str()[0] == '{') { //compound
			token t;
			t.type = token::COMPOUND;
			size_t end = text.find_first_of("}");
			if (end != std::string::npos) {
				t.text = text.substr(1, end-1);
				buffer.push_back(t);
			}
			else {
				throw std::string("parsing error: missing \"}\"");
			}
		}
		else if (text.size() > 0 && text.find_first_of("abcdefghijklmnopqrstuvwxyz") != std::string::npos) {
			token t;
			t.type = token::NAME;
			t.text = text;
			buffer.push_back(t);
		}
		else if (text.size() > 0 && text.find_first_of("0123456789") != std::string::npos) {
			token t;
			t.type = token::VALUE;
			t.text = text;
			buffer.push_back(t);
		}
	}
}

const std::string filter_to_string(vr::vr_log::Filter f){
	switch (f) {
	case vr::vr_log::F_AXES:
		return "AXES";
	case vr::vr_log::F_BUTTON:
		return "BUTTON";
	case vr::vr_log::F_HMD:
		return "HMD";
	case vr::vr_log::F_POSE:
		return "POSE";
	case vr::vr_log::F_VIBRATION:
		return "VIBRATION";
	default:
		return "UNKNOWN_FILTER";
	}
}

const std::unordered_map<std::string, vr::vr_log::Filter> filter_map = {
   {"AXES", vr::vr_log::F_AXES},
   {"BUTTON", vr::vr_log::F_BUTTON},
   {"HMD", vr::vr_log::F_HMD },
   {"POSE", vr::vr_log::F_POSE},
   {"VIBRATION", vr::vr_log::F_VIBRATION},
   {"UNKNOWN_FILTER", vr::vr_log::F_NONE}
};


const vr::vr_log::Filter filter_from_string(const std::string& f) {
	const auto it = filter_map.find(f);
	if (it != filter_map.cend()) {
		return (*it).second;
	}
	return vr::vr_log::F_NONE;
}


void vr::vr_log::disable_log()
{
	log_storage_mode = SM_NONE;
	if (log_stream) {
		log_stream = nullptr;
	}
}

void vr::vr_log::enable_in_memory_log()
{
	if (!setting_locked)
		log_storage_mode = log_storage_mode | SM_IN_MEMORY;
}

void vr::vr_log::enable_ostream_log(const std::shared_ptr<std::ostream>& stream)
{
	if (!setting_locked)
		log_stream = stream;
		log_stream->precision(std::numeric_limits<double>::max_digits10);
		log_storage_mode = log_storage_mode | SM_OSTREAM;
}

vr::vr_log::vr_log(std::istringstream& is) {
	load_state(is);
}

void vr::vr_log::log_vr_state(const vr::vr_kit_state& state, const int mode, const int filter, const double time,std::ostream* log_stream)
{
	if (!setting_locked)
		return;
	
	//time stamp
	if (mode & SM_IN_MEMORY) {
		this->time_stamp.push_back(time);
	}
	if (mode & SM_OSTREAM) {
		*(log_stream) << time;
	}
	if (mode != SM_NONE) {
		++nr_vr_states;
	}

	//controller state
	for (int ci = 0; ci < max_nr_controllers; ++ci) {
		controller_status[ci].push_back(state.controller[ci].status);
		if (mode & SM_IN_MEMORY) {
			if (filter & F_VIBRATION) {
				vec2 vibration = vec2(state.controller[ci].vibration[0], state.controller[ci].vibration[1]);
				this->controller_vibration[ci].push_back(vibration);
			}
			if (filter & F_AXES) {
				vecn axes(max_nr_controller_axes);
				for (int j = 0; j < max_nr_controller_axes; ++j) {
					axes(j) = state.controller[ci].axes[j];
				}
				this->controller_axes[ci].push_back(axes);
			}
			if (filter & F_POSE) {
				mat34 pose = mat34(3, 4, state.controller[ci].pose);
				this->controller_pose[ci].push_back(pose);
			}
			if (filter & F_BUTTON) {
				this->controller_button_flags[ci].push_back(state.controller[ci].button_flags);
			}
		}
		if (mode & SM_OSTREAM) {
			//<timestamp>, ({C <controller_id> [P <pose>] [B <button - mask>] [A <axes - state>] [V <vibration>] }, )* [H <pose>]
			//C{<controller_id> [P <pose>] [B <button - mask>] [A <axes - state>] [V <vibration>]}
			if (state.controller[ci].status == vr::VRStatus::VRS_TRACKED) {
				*(log_stream) << ",{C " << ci;
				if (filter & F_POSE) {
					*(log_stream) << " P";
					for (int j = 0; j < 12; ++j)
						*(log_stream) << ' ' << state.controller[ci].pose[j];
				}
				if (filter & F_BUTTON) {
					*(log_stream) << " B " << state.controller[ci].button_flags;
				}
				if (filter & F_AXES) {
					*(log_stream) << " A";
					for (int j = 0; j < max_nr_controller_axes; ++j)
						*(log_stream) << ' ' << state.controller[ci].axes[j];
				}
				if (filter & F_VIBRATION) {
					*(log_stream) << " V";
					for (int j = 0; j < 2; ++j)
						*(log_stream) << ' ' << state.controller[ci].vibration[j];
				}
				*(log_stream) << "}";
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
			if (state.hmd.status == vr::VRStatus::VRS_TRACKED) {
				*(log_stream) << ",{H ";
				for (int j = 0; j < 12; ++j)
					*(log_stream) << ' ' << state.hmd.pose[j];
				*(log_stream) << '}';
			}
		}
	}
	//end line
	if ((mode & SM_OSTREAM) && log_stream) {
		*(log_stream) << '\n';
	}
}

template <typename T,unsigned SIZE>
void parse_array(std::istringstream& line,T* storage) {
	for (int i = 0; i < SIZE; ++i) {
		line >> storage[i];
	}
}

int parse_filter_string(std::istringstream& line) {
	int filters = 0;
	while (!line.eof()) {
		std::string filter;
		line >> filter;
		filters |= filter_from_string(filter);
	}
	return filters;
}

//expects controller state string
unsigned parse_controller_state(std::istringstream& line, vr::vr_controller_state& state) {
	unsigned filter = 0;
	std::string cinfo_type;
	
	while (!line.eof()) {
		line >> cinfo_type;
		if (cinfo_type == "P") {
			filter |= vr::vr_log::F_POSE;
			parse_array<float, 12>(line, state.pose);
		}
		else if (cinfo_type == "A") {
			filter |= vr::vr_log::F_AXES;
			parse_array<float, vr::max_nr_controller_axes>(line, state.axes);
		}
		else if (cinfo_type == "B") {
			filter |= vr::vr_log::F_BUTTON;
			line >> state.button_flags;
		}
		else if (cinfo_type == "V") {
			filter |= vr::vr_log::F_VIBRATION;
			parse_array<float, 2>(line, state.vibration);
		}
	}
	return filter;
}

//expects up to 5 COMPOUND tokens, returns active filters found
template <typename iterator>
unsigned parse_vr_kit_state(iterator it, iterator last, vr::vr_kit_state& state,double& time) {
	unsigned filter = 0;

	while (it != last)
	{
		if (it->type == token::COMPOUND) {
			std::string type;
			std::istringstream line(it->text);
			line >> type;
			if (type == "C") { //controller info compund
				int cid = -1; //controller id
				line >> cid;
				if (cid >= 0 && cid < vr::max_nr_controllers)
					filter |= parse_controller_state(line, state.controller[cid]);
				else
					throw std::string("invalid controller id");
			}
			else if (type == "H") { //parse hmd info
				filter |= vr::vr_log::F_HMD;
				parse_array<float, 12>(line, state.hmd.pose);
			}
		}
		else {
			throw std::string("unexpected token");
			//unexpected token
		}
		++it;
	}
	return filter;
}


void vr::vr_log::lock_settings()
{
	setting_locked = true;
	//write header
	if (log_storage_mode & SM_OSTREAM) {
		*(log_stream) << "filters,{";
		int fil = 1;
		bool first = true;
		while (fil < F_ALL) {
			if (fil & filters) {
				if (!first) {
					*(log_stream) << " ";
				} else { 
					first = false;
				}
				*(log_stream) << filter_to_string(static_cast<vr::vr_log::Filter>(fil));
			}
			fil = fil << 1;
		}
		*(log_stream) << "}\n";
	}
}

bool vr::vr_log::load_state(std::istringstream& is) {
	//log lines look like this: 
	//<timestamp>, ({ C <controller_id>[P <pose>][B <button - mask>][A <axes - state>][V <vibration>] }, )* [H <pose>]
	if (setting_locked)
		return false;
	log_storage_mode = SM_IN_MEMORY;
	set_filter(F_ALL);
	lock_settings();
	//write log
	std::vector<token> tokens;
	bool found_filters = false;

	try {
		while (!is.eof()) {
			std::string line;
			std::getline(is, line);
			tokens.clear();
			tokenize(line, tokens);
			double time = -1;
			if (tokens.size()) {
				if (tokens[0].type == token::VALUE) {
					if (!found_filters) {
						return false;
					}
					double time = std::stod(tokens[0].text);
					vr::vr_kit_state state;
					parse_vr_kit_state(tokens.cbegin() + 1, tokens.cend(), state, time);
					log_vr_state(state, time);
				}
				else if (tokens[0].type == token::NAME) {
					if (tokens.size() >= 2 && tokens[0].text == "filters" && tokens[1].type == token::COMPOUND) {
						found_filters = true;
						auto text = std::istringstream(tokens[1].text);
						filters = parse_filter_string(text);
					}
				}
				else {
					throw std::string("parsing error expected time got " + tokens[0].text);
				}
			}
		}
	}
	catch (std::string err) {
		return false;
	}

	disable_log();
	return true;
}