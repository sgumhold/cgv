#include "event.h"

#include <cgv/utils/scan.h>
#include <iomanip>
namespace cgv {
	namespace gui {

#define MAX_MOD_LENGTH 5
static int nr_mods = 4;
static const char* mod_names[4] = { "Shift+", "Alt+", "Ctrl+", "Meta+" };
static EventModifier mod_ids[4] = { EM_SHIFT, EM_ALT,EM_CTRL, EM_META };

#define MAX_TOG_LENGTH 10
static int nr_togs = 3;
static const char* tog_names[3] = { "CapsLock", "NumLock", "ScrollLock" };
static EventToggleKeys tog_ids[3] = { ETK_CAPS_LOCK, ETK_NUM_LOCK, ETK_SCROLL_LOCK };


// construct event from its kind
event::event(unsigned int _kind, unsigned char _modifiers, unsigned char _toggle_keys, double _time)
	: kind(_kind), flags(0), modifiers(_modifiers), toggle_keys(_toggle_keys), time(_time) 
{
}

// virtual destructor for events
event::~event()
{
}

/// return the device id, by default returns 0
void* event::get_device_id() const
{
	return 0;
}

// convert a modifier combination into a readable string. 
std::string get_modifier_string(EventModifier modifiers)
{
	std::string res;
	for (int mi=0; mi<nr_mods; ++mi)
		if ( (modifiers&mod_ids[mi]) != 0)
			res += mod_names[mi];
	return res;
}

// convert a toggle key combination into a readable string separated by '+' signs, i.e. "CapsLock+NumLock"
std::string get_toggle_keys_string(EventToggleKeys toggle_keys)
{
	std::string res;
	for (int ti=0; ti<nr_togs; ++ti)
		if ( (toggle_keys&tog_ids[ti]) != 0) {
			if (!res.empty())
				res += '+';
			res += tog_names[ti];
		}
	return res;
}

unsigned char stream_in_toggle_keys(std::istream& is)
{
 	int c = is.peek();
	if (c > 255)
		return 0;
	c = cgv::utils::to_upper(c);
	int ti;
	for (ti = 0; ti < nr_togs; ++ti) 
		if (tog_names[ti][0] == c)
			break;
	if (ti == nr_togs)
		return 0;

	int n = (int)std::string(tog_names[ti]).size();
	int buffer[MAX_TOG_LENGTH];
	for (int i=1; i<n; ++i) {
		buffer[i-1] = is.get();
		c = is.peek();
		if (c > 255)
			return 0;
		c = cgv::utils::to_lower(c);
		if (tog_names[ti][i] != c) {
			for (int j = i; j > 0; ) {
				--j;
				is.putback(buffer[j]);
			}
			return 0;
		}
	}
	is.get();
	return tog_ids[ti];
}


unsigned char stream_in_modifier(std::istream& is)
{
 	int c = is.peek();
	if (c > 255)
		return 0;
	c = cgv::utils::to_upper(c);
	int mi;
	for (mi = 0; mi < nr_mods; ++mi) 
		if (mod_names[mi][0] == c)
			break;
	if (mi == nr_mods)
		return 0;

	int n = (int)std::string(mod_names[mi]).size();
	int buffer[MAX_MOD_LENGTH];
	for (int i=1; i<n; ++i) {
		buffer[i-1] = is.get();
		c = is.peek();
		if (c > 255)
			return 0;
		c = cgv::utils::to_lower(c);
		if (mod_names[mi][i] != c) {
			for (int j = i; j > 0; ) {
				--j;
				is.putback(buffer[j]);
			}
			return 0;
		}
	}
	is.get();
	return mod_ids[mi];
}

unsigned char stream_in_modifiers(std::istream& is)
{
	unsigned char modifiers = 0;
	do {
		unsigned char m = stream_in_modifier(is);
		if (m == 0)
			break;
		modifiers |= m;
	} while (true);
	return modifiers;
}

// write to stream
void event::stream_out(std::ostream& os) const
{
	const char* flag_strs[] = {
		"Multi", "Drag&Drop", "Gamepad", "VR"
	};
	EventFlags flags[] = {
		EF_MULTI,
		EF_DND, 
		EF_PAD,
		EF_VR
	};
	const char* kind_strs[] = { "none", "key", "mouse", "throttle", "stick", "pose" };

	os << kind_strs[kind] << "[" << std::fixed << std::showpoint << std::setprecision(3) << time << "] ";
	if (get_toggle_keys() != 0) {
		os << "(";
		bool last = false;
		if ((get_toggle_keys()&ETK_CAPS_LOCK)!=0) {
			os << "Caps_Lock";
			last = true;
		}
		if ((get_toggle_keys()&ETK_NUM_LOCK)!=0) {
			if (last)
				os << ",";
			os << "Num_Lock";
			last = true;
		}
		if ((get_toggle_keys()&ETK_SCROLL_LOCK)!=0) {
			if (last)
				os << ",";
			os << "Scroll_Lock";
			last = true;
		}
		os << ") ";
	}
	os << get_modifier_string(EventModifier(get_modifiers())).c_str();
	if (get_flags() != EF_NONE) {
		os << "{";
		bool first = true;
		for (unsigned i = 0; i < 4; ++i) {
			if ((get_flags()&flags[i]) != 0) {
				if (first)
					first = false;
				else
					os << '|';
				os << flag_strs[i];
			}
		}
		os << "}";
	}
}
// read from stream
void event::stream_in(std::istream&)
{
	std::cerr << "event::stream_in not implemented yet" << std::endl;
}

// set the kind of the event
void event::set_kind(unsigned char _kind)
{
	kind = _kind;
}
// return, what kind of event this is
unsigned int event::get_kind() const
{
	return kind;
}

/// set the event flags
void event::set_flags(unsigned char _flags)
{
	flags = _flags;
}

/// return the event flags
unsigned event::get_flags() const
{
	return flags;
}

// set the state of the toggle keys
void event::set_toggle_keys(unsigned char _toggle_keys)
{
	toggle_keys = _toggle_keys;
}

// return the state of the toggle keys
unsigned char event::get_toggle_keys() const
{
	return toggle_keys;
}

// set the modifiers
void event::set_modifiers(unsigned char _modifiers)
{
	modifiers = _modifiers;
}
// return the active modifiers
unsigned char event::get_modifiers() const
{
	return modifiers;
}
// set the time of the event
void event::set_time(const double& _time)
{
	time = _time;
}
// return the time of the event in seconds
double event::get_time() const
{
	return time;
}

unsigned char& ref_current_modifiers()
{
	static unsigned char mods = 0;
	return mods;
}


	}
}

#include <cgv/config/lib_end.h>